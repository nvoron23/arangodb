////////////////////////////////////////////////////////////////////////////////
/// @brief Aql, expression
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2014 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Jan Steemann
/// @author Copyright 2014, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2012-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "Aql/Expression.h"
#include "Aql/AqlItemBlock.h"
#include "Aql/AqlValue.h"
#include "Aql/Ast.h"
#include "Aql/AttributeAccessor.h"
#include "Aql/Executor.h"
#include "Aql/V8Expression.h"
#include "Aql/Variable.h"
#include "Basics/Exceptions.h"
#include "Basics/JsonHelper.h"
#include "Basics/StringBuffer.h"
#include "Basics/json.h"
#include "ShapedJson/shaped-json.h"
#include "VocBase/document-collection.h"

using namespace triagens::aql;
using Json = triagens::basics::Json;
using JsonHelper = triagens::basics::JsonHelper;

// -----------------------------------------------------------------------------
// --SECTION--                                             public static members
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief "constant" global object for NULL which can be shared by all 
/// expressions but must never be freed
////////////////////////////////////////////////////////////////////////////////

TRI_json_t const Expression::NullJson  = { TRI_JSON_NULL, { false } };

////////////////////////////////////////////////////////////////////////////////
/// @brief "constant" global object for TRUE which can be shared by all 
/// expressions but must never be freed
////////////////////////////////////////////////////////////////////////////////

TRI_json_t const Expression::TrueJson  = { TRI_JSON_BOOLEAN, { true } };

////////////////////////////////////////////////////////////////////////////////
/// @brief "constant" global object for FALSE which can be shared by all 
/// expressions but must never be freed
////////////////////////////////////////////////////////////////////////////////

TRI_json_t const Expression::FalseJson = { TRI_JSON_BOOLEAN, { false } };

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create the expression
////////////////////////////////////////////////////////////////////////////////

Expression::Expression (Ast* ast,
                        AstNode const* node)
  : _ast(ast),
    _executor(_ast->query()->executor()),
    _node(node),
    _type(UNPROCESSED),
    _canThrow(true),
    _canRunOnDBServer(false),
    _isDeterministic(false),
    _hasDeterminedAttributes(false),
    _built(false),
    _attributes(),
    _buffer(TRI_UNKNOWN_MEM_ZONE) {

  TRI_ASSERT(_ast != nullptr);
  TRI_ASSERT(_executor != nullptr);
  TRI_ASSERT(_node != nullptr);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create an expression from JSON
////////////////////////////////////////////////////////////////////////////////

Expression::Expression (Ast* ast,
                        triagens::basics::Json const& json)
  : Expression(ast, new AstNode(ast, json.get("expression"))) {

}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the expression
////////////////////////////////////////////////////////////////////////////////

Expression::~Expression () {
  if (_built) {
    switch (_type) {
      case JSON:
        TRI_ASSERT(_data != nullptr);
        TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, _data);
        break;

      case ATTRIBUTE: {
        TRI_ASSERT(_accessor != nullptr);
        delete _accessor;
        break;
      }

      case V8:
        delete _func;
        break;
      
      case SIMPLE: 
      case UNPROCESSED: {
        // nothing to do
        break;
      }
      
    }
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief return all variables used in the expression
////////////////////////////////////////////////////////////////////////////////

std::unordered_set<Variable*> Expression::variables () const {
  return Ast::getReferencedVariables(_node);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief execute the expression
////////////////////////////////////////////////////////////////////////////////

AqlValue Expression::execute (triagens::arango::AqlTransaction* trx,
                              AqlItemBlock const* argv,
                              size_t startPos,
                              std::vector<Variable*> const& vars,
                              std::vector<RegisterId> const& regs,
                              TRI_document_collection_t const** collection) {

  if (! _built) {
    buildExpression();
  }

  TRI_ASSERT(_type != UNPROCESSED);
  TRI_ASSERT(_built);

  // and execute
  switch (_type) {
    case JSON: {
      TRI_ASSERT(_data != nullptr);
      return AqlValue(new Json(TRI_UNKNOWN_MEM_ZONE, _data, Json::NOFREE));
    }

    case SIMPLE: {
      return executeSimpleExpression(_node, collection, trx, argv, startPos, vars, regs, true);
    }

    case ATTRIBUTE: {
      TRI_ASSERT(_accessor != nullptr);
      return _accessor->get(trx, argv, startPos, vars, regs);
    }
    
    case V8: {
      TRI_ASSERT(_func != nullptr);
      try {
        ISOLATE;
        // Dump the expression in question  
        // std::cout << triagens::basics::Json(TRI_UNKNOWN_MEM_ZONE, _node->toJson(TRI_UNKNOWN_MEM_ZONE, true)).toString()<< "\n";
        return _func->execute(isolate, _ast->query(), trx, argv, startPos, vars, regs);
      }
      catch (triagens::basics::Exception& ex) {
        if (_ast->query()->verboseErrors()) {
          ex.addToMessage(" while evaluating expression ");
          auto json = _node->toJson(TRI_UNKNOWN_MEM_ZONE, false);

          if (json != nullptr) {
            ex.addToMessage(triagens::basics::JsonHelper::toString(json));
            TRI_Free(TRI_UNKNOWN_MEM_ZONE, json);
          }
        }
        throw;
      }
    }

    case UNPROCESSED: {
      // fall-through to exception
    }
  }
      
  THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_INTERNAL, "invalid simple expression");
}

////////////////////////////////////////////////////////////////////////////////
/// @brief replace variables in the expression with other variables
////////////////////////////////////////////////////////////////////////////////

void Expression::replaceVariables (std::unordered_map<VariableId, Variable const*> const& replacements) {
  _node = _ast->clone(_node);
  TRI_ASSERT(_node != nullptr);

  _node = _ast->replaceVariables(const_cast<AstNode*>(_node), replacements);
  invalidate(); 
}

////////////////////////////////////////////////////////////////////////////////
/// @brief replace a variable reference in the expression with another
/// expression (e.g. inserting c = `a + b` into expression `c + 1` so the latter 
/// becomes `a + b + 1`
////////////////////////////////////////////////////////////////////////////////

void Expression::replaceVariableReference (Variable const* variable, 
                                           AstNode const* node) {
  _node = _ast->clone(_node);
  TRI_ASSERT(_node != nullptr);

  _node = _ast->replaceVariableReference(const_cast<AstNode*>(_node), variable, node);
  invalidate(); 

  if (_type == ATTRIBUTE) {
    if (_built) {
      delete _accessor;
      _accessor = nullptr;
      _built = false;
    }
    // must even set back the expression type so the expression will be analyzed again
    _type = UNPROCESSED;
  }

  const_cast<AstNode*>(_node)->clearFlags();
  _attributes.clear();
  _hasDeterminedAttributes = false;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief invalidates an expression
/// this only has an effect for V8-based functions, which need to be created,
/// used and destroyed in the same context. when a V8 function is used across
/// multiple V8 contexts, it must be invalidated in between
////////////////////////////////////////////////////////////////////////////////

void Expression::invalidate () {
  if (_type == V8) {
    // V8 expressions need a special handling
    if (_built) {
      delete _func;
      _func = nullptr;
      _built = false;
    }
  }
  // we do not need to invalidate the other expression type 
  // expression data will be freed in the destructor
}

// -----------------------------------------------------------------------------
// --SECTION--                                                 private functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief find a value in an AQL list node
/// this performs either a binary search (if the node is sorted) or a
/// linear search (if the node is not sorted)
////////////////////////////////////////////////////////////////////////////////

bool Expression::findInArray (AqlValue const& left, 
                              AqlValue const& right, 
                              TRI_document_collection_t const* leftCollection, 
                              TRI_document_collection_t const* rightCollection,
                              triagens::arango::AqlTransaction* trx,
                              AstNode const* node) const {
  TRI_ASSERT_EXPENSIVE(right.isArray());
 
  size_t const n = right.arraySize();

  if (node->getMember(1)->isSorted()) {
    // node values are sorted. can use binary search
    size_t l = 0;
    size_t r = n - 1;

    while (true) {
      // determine midpoint
      size_t m = l + ((r - l) / 2);
      auto listItem = right.extractArrayMember(trx, rightCollection, m, false);
      AqlValue listItemValue(&listItem);

      int compareResult = AqlValue::Compare(trx, left, leftCollection, listItemValue, nullptr, false);

      if (compareResult == 0) {
        // item found in the list
        return true;
      }

      if (compareResult < 0) {
        if (m == 0) {
          // not found
          return false;
        }
        r = m - 1;
      }
      else {
        l = m + 1;
      }
      if (r < l) {
        return false;
      }
    }
  }
  else {
    // use linear search
    for (size_t i = 0; i < n; ++i) {
      // do not copy the list element we're looking at
      auto listItem = right.extractArrayMember(trx, rightCollection, i, false);
      AqlValue listItemValue(&listItem);

      int compareResult = AqlValue::Compare(trx, left, leftCollection, listItemValue, nullptr, false);

      if (compareResult == 0) {
        // item found in the list
        return true;
      }
    }
    
    return false;  
  }
} 

////////////////////////////////////////////////////////////////////////////////
/// @brief analyze the expression (determine its type etc.)
////////////////////////////////////////////////////////////////////////////////

void Expression::analyzeExpression () {
  TRI_ASSERT(_type == UNPROCESSED);
  TRI_ASSERT(_built == false);

  if (_node->isConstant()) {
    // expression is a constant value
    _type             = JSON;
    _canThrow         = false;
    _canRunOnDBServer = true;
    _isDeterministic  = true;
    _data             = nullptr;
  }
  else if (_node->isSimple()) {
    // expression is a simple expression
    _type             = SIMPLE;
    _canThrow         = _node->canThrow();
    _canRunOnDBServer = _node->canRunOnDBServer();
    _isDeterministic  = _node->isDeterministic();

    if (_node->type == NODE_TYPE_ATTRIBUTE_ACCESS) {
      TRI_ASSERT_EXPENSIVE(_node->numMembers() == 1);
      auto member = _node->getMemberUnchecked(0);
      std::vector<char const*> parts{ static_cast<char const*>(_node->getData()) };

      while (member->type == NODE_TYPE_ATTRIBUTE_ACCESS) {
        parts.insert(parts.begin(), static_cast<char const*>(member->getData()));
        member = member->getMemberUnchecked(0);
      }

      if (member->type == NODE_TYPE_REFERENCE) {
        auto v = static_cast<Variable const*>(member->getData());

        // specialize the simple expression into an attribute accessor
        _accessor = new AttributeAccessor(parts, v);
        _type = ATTRIBUTE;
        _built = true;
      }
    }
  }
  else {
    // expression is a V8 expression
    _type             = V8;
    _canThrow         = _node->canThrow();
    _canRunOnDBServer = _node->canRunOnDBServer();
    _isDeterministic  = _node->isDeterministic();
    _func             = nullptr;

    if (! _hasDeterminedAttributes) {
      // determine all top-level attributes used in expression only once
      // as this might be expensive
      _hasDeterminedAttributes = true;

      bool isSafeForOptimization;
      _attributes = std::move(Ast::getReferencedAttributes(_node, isSafeForOptimization));

      if (! isSafeForOptimization) {
        _attributes.clear();
        // unfortunately there are not only top-level attribute accesses but
        // also other accesses, e.g. the index values or accesses of the whole value.
        // for example, we cannot optimize LET x = a +1 or LET x = a[0], but LET x = a._key 
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief build the expression
////////////////////////////////////////////////////////////////////////////////

void Expression::buildExpression () {
  TRI_ASSERT(! _built);

  if (_type == UNPROCESSED) {
    analyzeExpression();
  }

  if (_type == JSON) {
    TRI_ASSERT(_data == nullptr);
    // generate a constant value
    _data = _node->toJsonValue(TRI_UNKNOWN_MEM_ZONE);

    if (_data == nullptr) {
      THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_INTERNAL, "invalid json in simple expression");
    }
  }
  else if (_type == V8) {
    // generate a V8 expression
    _func = _executor->generateExpression(_node);
  
    // optimizations for the generated function 
    if (_func != nullptr && ! _attributes.empty()) {
      // pass which variables do not need to be fully constructed
      _func->setAttributeRestrictions(_attributes);
    }
  }

  _built = true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief execute an expression of type SIMPLE, the convention is that
/// the resulting AqlValue will be destroyed outside eventually
////////////////////////////////////////////////////////////////////////////////

AqlValue Expression::executeSimpleExpression (AstNode const* node,
                                              TRI_document_collection_t const** collection, 
                                              triagens::arango::AqlTransaction* trx,
                                              AqlItemBlock const* argv,
                                              size_t startPos,
                                              std::vector<Variable*> const& vars,
                                              std::vector<RegisterId> const& regs,
                                              bool doCopy) {
  if (node->type == NODE_TYPE_ATTRIBUTE_ACCESS) {
    // object lookup, e.g. users.name
    TRI_ASSERT_EXPENSIVE(node->numMembers() == 1);

    auto member = node->getMemberUnchecked(0);
    auto name = static_cast<char const*>(node->getData());

    TRI_document_collection_t const* myCollection = nullptr;
    AqlValue result = executeSimpleExpression(member, &myCollection, trx, argv, startPos, vars, regs, false);

    auto j = result.extractObjectMember(trx, myCollection, name, true, _buffer);
    result.destroy();
    return AqlValue(new Json(TRI_UNKNOWN_MEM_ZONE, j.steal()));
  }
  
  else if (node->type == NODE_TYPE_INDEXED_ACCESS) {
    // array lookup, e.g. users[0]
    // note: it depends on the type of the value whether an array lookup or an object lookup is performed
    // for example, if the value is an object, then its elements might be accessed like this:
    // users['name'] or even users['0'] (as '0' is a valid attribute name, too)
    // if the value is an array, then string indexes might also be used and will be converted to integers, e.g.
    // users['0'] is the same as users[0], users['-2'] is the same as users[-2] etc.
    TRI_ASSERT(node->numMembers() == 2);

    auto member = node->getMember(0);
    auto index = node->getMember(1);

    TRI_document_collection_t const* myCollection = nullptr;
    AqlValue result = executeSimpleExpression(member, &myCollection, trx, argv, startPos, vars, regs, false);

    if (result.isArray()) {
      TRI_document_collection_t const* myCollection2 = nullptr;
      AqlValue indexResult = executeSimpleExpression(index, &myCollection2, trx, argv, startPos, vars, regs, false);

      if (indexResult.isNumber()) {
        auto j = result.extractArrayMember(trx, myCollection, indexResult.toInt64(), true);
        indexResult.destroy();
        result.destroy();
        return AqlValue(new Json(TRI_UNKNOWN_MEM_ZONE, j.steal()));
      }
      else if (indexResult.isString()) {
        auto&& value = indexResult.toString();
        indexResult.destroy();

        try {
          // stoll() might throw an exception if the string is not a number
          int64_t position = static_cast<int64_t>(std::stoll(value.c_str()));
          auto j = result.extractArrayMember(trx, myCollection, position, true);
          result.destroy();
          return AqlValue(new Json(TRI_UNKNOWN_MEM_ZONE, j.steal()));
        }
        catch (...) {
          // no number found. 
        }
      }
      else {
        indexResult.destroy();
      }

      // fall-through to returning null
    }
    else if (result.isObject()) {
      TRI_document_collection_t const* myCollection2 = nullptr;
      AqlValue indexResult = executeSimpleExpression(index, &myCollection2, trx, argv, startPos, vars, regs, false);

      if (indexResult.isNumber()) {
        auto&& indexString = std::to_string(indexResult.toInt64());
        auto j = result.extractObjectMember(trx, myCollection, indexString.c_str(), true, _buffer);
        indexResult.destroy();
        result.destroy();
        return AqlValue(new Json(TRI_UNKNOWN_MEM_ZONE, j.steal()));
      }
      else if (indexResult.isString()) {
        auto&& value = indexResult.toString();
        indexResult.destroy();

        auto j = result.extractObjectMember(trx, myCollection, value.c_str(), true, _buffer);
        result.destroy();
        return AqlValue(new Json(TRI_UNKNOWN_MEM_ZONE, j.steal()));
      }
      else {
        indexResult.destroy();
      }
      // fall-through to returning null
    }
    result.destroy();
      
    return AqlValue(new Json(TRI_UNKNOWN_MEM_ZONE, &NullJson, Json::NOFREE));
  }
  
  else if (node->type == NODE_TYPE_ARRAY) {
    if (node->isConstant()) {
      auto json = node->computeJson();

      if (json == nullptr) {
        THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
      }

      // we do not own the JSON but the node does!
      return AqlValue(new Json(TRI_UNKNOWN_MEM_ZONE, json, Json::NOFREE));
    }

    size_t const n = node->numMembers();
    std::unique_ptr<Json> array(new Json(Json::Array, n));

    for (size_t i = 0; i < n; ++i) {
      auto member = node->getMemberUnchecked(i);
      TRI_document_collection_t const* myCollection = nullptr;

      AqlValue result = executeSimpleExpression(member, &myCollection, trx, argv, startPos, vars, regs, false);
      array->add(result.toJson(trx, myCollection));
      result.destroy();
    }

    return AqlValue(array.release());
  }

  else if (node->type == NODE_TYPE_OBJECT) {
    if (node->isConstant()) {
      auto json = node->computeJson();

      if (json == nullptr) {
        THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
      }

      // we do not own the JSON but the node does!
      return AqlValue(new Json(TRI_UNKNOWN_MEM_ZONE, json, Json::NOFREE));
    }

    size_t const n = node->numMembers();
    std::unique_ptr<Json> object(new Json(Json::Object, n));

    for (size_t i = 0; i < n; ++i) {
      auto member = node->getMemberUnchecked(i);
      TRI_document_collection_t const* myCollection = nullptr;

      TRI_ASSERT(member->type == NODE_TYPE_OBJECT_ELEMENT);
      auto key = member->getStringValue();
      member = member->getMember(0);

      AqlValue result = executeSimpleExpression(member, &myCollection, trx, argv, startPos, vars, regs, false);
      object->set(key, result.toJson(trx, myCollection));
      result.destroy();
    }
    return AqlValue(object.release());
  }

  else if (node->type == NODE_TYPE_VALUE) {
    auto json = node->computeJson();

    if (json == nullptr) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
    }

    // we do not own the JSON but the node does!
    return AqlValue(new Json(TRI_UNKNOWN_MEM_ZONE, json, Json::NOFREE)); 
  }

  else if (node->type == NODE_TYPE_REFERENCE) {
    auto v = static_cast<Variable const*>(node->getData());

    size_t i = 0;
    for (auto it = vars.begin(); it != vars.end(); ++it, ++i) {
      if ((*it)->name == v->name) {
        TRI_ASSERT(collection != nullptr);

        // save the collection info
        *collection = argv->getDocumentCollection(regs[i]); 
        if (doCopy) {
          return argv->getValueReference(startPos, regs[i]).clone();
        }
        
        // AqlValue.destroy() will be called for the returned value soon,
        // so we must not return the original AqlValue from the AqlItemBlock here 
        return argv->getValueReference(startPos, regs[i]).shallowClone();
      }
    }
    // fall-through to exception
  }
  
  else if (node->type == NODE_TYPE_FCALL) {
    // some functions have C++ handlers
    // check if the called function has one
    auto func = static_cast<Function*>(node->getData());
    TRI_ASSERT(func->implementation != nullptr);

    TRI_document_collection_t const* myCollection = nullptr;
    auto member = node->getMemberUnchecked(0);
    TRI_ASSERT(member->type == NODE_TYPE_ARRAY);

    AqlValue result = executeSimpleExpression(member, &myCollection, trx, argv, startPos, vars, regs, false);
       
    try { 
      auto res2 = func->implementation(_ast->query(), trx, myCollection, result);
      result.destroy();
      return res2;
    }
    catch (...) {
      // prevent leak and rethrow error
      result.destroy();
      throw; 
    }
  }

  else if (node->type == NODE_TYPE_RANGE) {
    TRI_document_collection_t const* leftCollection = nullptr;
    TRI_document_collection_t const* rightCollection = nullptr;

    auto low = node->getMember(0);
    auto high = node->getMember(1);
    AqlValue resultLow  = executeSimpleExpression(low, &leftCollection, trx, argv, startPos, vars, regs, false);
    AqlValue resultHigh = executeSimpleExpression(high, &rightCollection, trx, argv, startPos, vars, regs, false);
    AqlValue res = AqlValue(resultLow.toInt64(), resultHigh.toInt64());
    resultLow.destroy();
    resultHigh.destroy();

    return res;
  }
  
  else if (node->type == NODE_TYPE_OPERATOR_UNARY_NOT) {
    TRI_document_collection_t const* myCollection = nullptr;
    AqlValue operand = executeSimpleExpression(node->getMember(0), &myCollection, trx, argv, startPos, vars, regs, false);
    
    bool const operandIsTrue = operand.isTrue();
    operand.destroy();
    return AqlValue(new Json(TRI_UNKNOWN_MEM_ZONE, operandIsTrue ? &FalseJson : &TrueJson, Json::NOFREE));
  }
  
  else if (node->type == NODE_TYPE_OPERATOR_BINARY_AND ||
           node->type == NODE_TYPE_OPERATOR_BINARY_OR) {
    TRI_document_collection_t const* leftCollection = nullptr;
    AqlValue left  = executeSimpleExpression(node->getMember(0), &leftCollection, trx, argv, startPos, vars, regs, true);
    TRI_document_collection_t const* rightCollection = nullptr;
    AqlValue right = executeSimpleExpression(node->getMember(1), &rightCollection, trx, argv, startPos, vars, regs, true);
    
    if (node->type == NODE_TYPE_OPERATOR_BINARY_AND) {
      // AND
      if (left.isTrue()) {
        // left is true => return right
        left.destroy();
        return right;
      }

      // left is false, return left
      right.destroy();
      return left;
    }
    else {
      // OR
      if (left.isTrue()) {
        // left is true => return left
        right.destroy();
        return left;
      }

      // left is false => return right
      left.destroy();
      return right;
    }
  }
  
  else if (node->type == NODE_TYPE_OPERATOR_BINARY_EQ ||
           node->type == NODE_TYPE_OPERATOR_BINARY_NE ||
           node->type == NODE_TYPE_OPERATOR_BINARY_LT ||
           node->type == NODE_TYPE_OPERATOR_BINARY_LE ||
           node->type == NODE_TYPE_OPERATOR_BINARY_GT ||
           node->type == NODE_TYPE_OPERATOR_BINARY_GE ||
           node->type == NODE_TYPE_OPERATOR_BINARY_IN ||
           node->type == NODE_TYPE_OPERATOR_BINARY_NIN) {
    TRI_document_collection_t const* leftCollection = nullptr;
    AqlValue left  = executeSimpleExpression(node->getMember(0), &leftCollection, trx, argv, startPos, vars, regs, false);
    TRI_document_collection_t const* rightCollection = nullptr;
    AqlValue right = executeSimpleExpression(node->getMember(1), &rightCollection, trx, argv, startPos, vars, regs, false);

    if (node->type == NODE_TYPE_OPERATOR_BINARY_IN ||
        node->type == NODE_TYPE_OPERATOR_BINARY_NIN) {
      // IN and NOT IN
      if (! right.isArray()) {
        // right operand must be a list, otherwise we return false
        left.destroy();
        right.destroy();
        // do not throw, but return "false" instead
        return AqlValue(new Json(TRI_UNKNOWN_MEM_ZONE, &FalseJson, Json::NOFREE));
      }
   
      bool result = findInArray(left, right, leftCollection, rightCollection, trx, node); 

      if (node->type == NODE_TYPE_OPERATOR_BINARY_NIN) {
        // revert the result in case of a NOT IN
        result = ! result;
      }
       
      left.destroy();
      right.destroy();
    
      return AqlValue(new triagens::basics::Json(result));
    }

    // all other comparison operators...

    // for equality and non-equality we can use a binary comparison
    bool compareUtf8 = (node->type != NODE_TYPE_OPERATOR_BINARY_EQ && node->type != NODE_TYPE_OPERATOR_BINARY_NE);

    int compareResult = AqlValue::Compare(trx, left, leftCollection, right, rightCollection, compareUtf8);
    left.destroy();
    right.destroy();

    if (node->type == NODE_TYPE_OPERATOR_BINARY_EQ) {
      return AqlValue(new Json(TRI_UNKNOWN_MEM_ZONE, (compareResult == 0) ? &TrueJson : &FalseJson, Json::NOFREE));
    }
    else if (node->type == NODE_TYPE_OPERATOR_BINARY_NE) {
      return AqlValue(new Json(TRI_UNKNOWN_MEM_ZONE, (compareResult != 0) ? &TrueJson : &FalseJson, Json::NOFREE));
    }
    else if (node->type == NODE_TYPE_OPERATOR_BINARY_LT) {
      return AqlValue(new Json(TRI_UNKNOWN_MEM_ZONE, (compareResult < 0) ? &TrueJson : &FalseJson, Json::NOFREE));
    }
    else if (node->type == NODE_TYPE_OPERATOR_BINARY_LE) {
      return AqlValue(new Json(TRI_UNKNOWN_MEM_ZONE, (compareResult <= 0) ? &TrueJson : &FalseJson, Json::NOFREE));
    }
    else if (node->type == NODE_TYPE_OPERATOR_BINARY_GT) {
      return AqlValue(new Json(TRI_UNKNOWN_MEM_ZONE, (compareResult > 0) ? &TrueJson : &FalseJson, Json::NOFREE));
    }
    else if (node->type == NODE_TYPE_OPERATOR_BINARY_GE) {
      return AqlValue(new Json(TRI_UNKNOWN_MEM_ZONE, (compareResult >= 0) ? &TrueJson : &FalseJson, Json::NOFREE));
    }
    // fall-through intentional
  }
  
  else if (node->type == NODE_TYPE_OPERATOR_TERNARY) {
    TRI_document_collection_t const* myCollection = nullptr;
    AqlValue condition  = executeSimpleExpression(node->getMember(0), &myCollection, trx, argv, startPos, vars, regs, false);

    bool const isTrue = condition.isTrue();
    condition.destroy();
    if (isTrue) {
      // return true part
      return executeSimpleExpression(node->getMember(1), &myCollection, trx, argv, startPos, vars, regs, true);
    }
    
    // return false part  
    return executeSimpleExpression(node->getMember(2), &myCollection, trx, argv, startPos, vars, regs, true);
  }
 
  THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_INTERNAL, "unhandled type in simple expression");
}

////////////////////////////////////////////////////////////////////////////////
/// @brief check whether this is an attribute access of any degree (e.g. a.b, 
/// a.b.c, ...)
////////////////////////////////////////////////////////////////////////////////

bool Expression::isAttributeAccess () const {
  auto expNode = _node;

  if (expNode->type != triagens::aql::NODE_TYPE_ATTRIBUTE_ACCESS) {
    return false;
  }

  while (expNode->type == triagens::aql::NODE_TYPE_ATTRIBUTE_ACCESS) {
    expNode = expNode->getMember(0);
  }
  
  return (expNode->type == triagens::aql::NODE_TYPE_REFERENCE);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief check whether this is a reference access
////////////////////////////////////////////////////////////////////////////////

bool Expression::isReference () const {
  return (_node->type == triagens::aql::NODE_TYPE_REFERENCE);
}
 
////////////////////////////////////////////////////////////////////////////////
/// @brief check whether this is a constant node
////////////////////////////////////////////////////////////////////////////////

bool Expression::isConstant () const {
  return _node->isConstant();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief this gives you ("variable.access", "Reference")
/// call isAttributeAccess in advance to ensure no exceptions.
////////////////////////////////////////////////////////////////////////////////

std::pair<std::string, std::string> Expression::getAttributeAccess () {
  if (_type == UNPROCESSED) {
    analyzeExpression();
  }

  if (_type != SIMPLE && 
      _type != ATTRIBUTE) {
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_INTERNAL,
                                   "getMultipleAttributes works only on simple expressions or attribute accesses!");
  }

  auto expNode = _node;
  std::vector<std::string> attributeVector;

  if (expNode->type != triagens::aql::NODE_TYPE_ATTRIBUTE_ACCESS) {
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_INTERNAL,
                                   "getMultipleAttributes works only on simple expressions or attribute accesses(2)!");
  }

  while (expNode->type == triagens::aql::NODE_TYPE_ATTRIBUTE_ACCESS) {
    attributeVector.emplace_back(std::string(expNode->getStringValue(), expNode->getStringLength()));
    expNode = expNode->getMember(0);
  }
  
  std::string attributeVectorStr;
  for (auto oneAttr = attributeVector.rbegin();
       oneAttr != attributeVector.rend();
       ++oneAttr) {
    if (! attributeVectorStr.empty()) {
      attributeVectorStr.push_back('.');
    }
    attributeVectorStr += *oneAttr;
  }

  if (expNode->type != triagens::aql::NODE_TYPE_REFERENCE) {
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_INTERNAL,
                                   "getMultipleAttributes works only on simple expressions or attribute accesses(3)!");
  }

  auto variable = static_cast<Variable*>(expNode->getData());

  return std::make_pair(variable->name, attributeVectorStr);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief stringify an expression
/// note that currently stringification is only supported for certain node types
////////////////////////////////////////////////////////////////////////////////

void Expression::stringify (triagens::basics::StringBuffer* buffer) const {
  _node->stringify(buffer, true, false);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief stringify an expression
/// note that currently stringification is only supported for certain node types
////////////////////////////////////////////////////////////////////////////////

void Expression::stringifyIfNotTooLong (triagens::basics::StringBuffer* buffer) const {
  _node->stringify(buffer, true, true);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
