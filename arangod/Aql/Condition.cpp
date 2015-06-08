////////////////////////////////////////////////////////////////////////////////
/// @brief Aql, condition 
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

#include "Condition.h"
#include "Aql/Ast.h"
#include "Aql/AstNode.h"
#include "Aql/Variable.h"
#include "Basics/Exceptions.h"
#include "Basics/json.h"
#include "Basics/JsonHelper.h"

using namespace triagens::aql;

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create the condition
////////////////////////////////////////////////////////////////////////////////

Condition::Condition (Ast* ast)
  : _ast(ast),
    _root(nullptr) {

}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the condition
////////////////////////////////////////////////////////////////////////////////

Condition::~Condition () {
  // memory for nodes is not owned and thus not freed by the condition
  // all nodes belong to the AST
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief add a sub-condition to the condition
/// the sub-condition will be AND-combined with the existing condition(s)
////////////////////////////////////////////////////////////////////////////////

void Condition::andCombine (AstNode const* node) {
  if (_root == nullptr) {
    // condition was empty before
    _root = _ast->clone(node);
  }
  else {
    // condition was not empty before, now AND-merge
    _root = _ast->createNodeBinaryOperator(NODE_TYPE_OPERATOR_BINARY_AND, _root, _ast->clone(node));
  }

  TRI_ASSERT(_root != nullptr);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief normalize the condition
////////////////////////////////////////////////////////////////////////////////

void Condition::normalize () {
  std::function<AstNode*(AstNode*)> transform = [this, &transform] (AstNode* node) -> AstNode* {
    if (node == nullptr) {
      return nullptr;
    }

    if (node->type == NODE_TYPE_OPERATOR_BINARY_AND ||
        node->type == NODE_TYPE_OPERATOR_BINARY_OR) {
      // convert binary AND/OR into n-ary AND/OR
      auto lhs = node->getMemberUnchecked(0);
      auto rhs = node->getMemberUnchecked(1);
      node = _ast->createNodeBinaryOperator(Ast::NaryOperatorType(node->type), lhs, rhs);
    }

    TRI_ASSERT(node->type != NODE_TYPE_OPERATOR_BINARY_AND &&
               node->type != NODE_TYPE_OPERATOR_BINARY_OR);


    if (node->type == NODE_TYPE_OPERATOR_NARY_AND) {
      // first recurse into subnodes
      node->changeMember(0, transform(node->getMemberUnchecked(0)));
      node->changeMember(1, transform(node->getMemberUnchecked(1)));

      auto lhs = node->getMemberUnchecked(0);
      auto rhs = node->getMemberUnchecked(1);

      if (lhs->type == NODE_TYPE_OPERATOR_NARY_OR &&
          rhs->type == NODE_TYPE_OPERATOR_NARY_OR) {
        auto and1 = transform(_ast->createNodeBinaryOperator(NODE_TYPE_OPERATOR_NARY_AND, lhs->getMemberUnchecked(0), rhs->getMemberUnchecked(0)));
        auto and2 = transform(_ast->createNodeBinaryOperator(NODE_TYPE_OPERATOR_NARY_AND, lhs->getMemberUnchecked(0), rhs->getMemberUnchecked(1)));
        auto or1  = _ast->createNodeBinaryOperator(NODE_TYPE_OPERATOR_NARY_OR, and1, and2);

        auto and3 = transform(_ast->createNodeBinaryOperator(NODE_TYPE_OPERATOR_NARY_AND, lhs->getMemberUnchecked(1), rhs->getMemberUnchecked(0)));
        auto and4 = transform(_ast->createNodeBinaryOperator(NODE_TYPE_OPERATOR_NARY_AND, lhs->getMemberUnchecked(1), rhs->getMemberUnchecked(1)));
        auto or2  = _ast->createNodeBinaryOperator(NODE_TYPE_OPERATOR_NARY_OR, and3, and4);

        return _ast->createNodeBinaryOperator(NODE_TYPE_OPERATOR_NARY_OR, or1, or2);
      }
      else if (lhs->type == NODE_TYPE_OPERATOR_NARY_OR) {
        auto and1 = transform(_ast->createNodeBinaryOperator(NODE_TYPE_OPERATOR_NARY_AND, rhs, lhs->getMemberUnchecked(0)));
        auto and2 = transform(_ast->createNodeBinaryOperator(NODE_TYPE_OPERATOR_NARY_AND, rhs, lhs->getMemberUnchecked(1)));

        return _ast->createNodeBinaryOperator(NODE_TYPE_OPERATOR_NARY_OR, and1, and2);
      }
      else if (rhs->type == NODE_TYPE_OPERATOR_NARY_OR) {
        auto and1 = transform(_ast->createNodeBinaryOperator(NODE_TYPE_OPERATOR_NARY_AND, lhs, rhs->getMemberUnchecked(0)));
        auto and2 = transform(_ast->createNodeBinaryOperator(NODE_TYPE_OPERATOR_NARY_AND, lhs, rhs->getMemberUnchecked(1)));

        return _ast->createNodeBinaryOperator(NODE_TYPE_OPERATOR_NARY_OR, and1, and2);
      }
    }
    else if (node->type == NODE_TYPE_OPERATOR_NARY_OR) {
      // first recurse into subnodes
      node->changeMember(0, transform(node->getMemberUnchecked(0)));
      node->changeMember(1, transform(node->getMemberUnchecked(1)));
    }

    return node;
  };
  
  // collapse function. 
  // will collapse nested logical AND/OR nodes 
  std::function<AstNode*(AstNode*)> collapse = [this, &collapse] (AstNode* node) -> AstNode* {
    if (node == nullptr) {
      return nullptr;
    }

    if (node->type == NODE_TYPE_OPERATOR_NARY_AND || 
        node->type == NODE_TYPE_OPERATOR_NARY_OR) {
      // first recurse into subnodes
      size_t const n = node->numMembers();

      for (size_t i = 0; i < n; ++i) {
        auto sub = collapse(node->getMemberUnchecked(i));

        if (sub->type == node->type) {
          // sub-node has the same type as parent node
          // now merge the sub-nodes of the sub-node into the parent node
          for (size_t j = 0; j < sub->numMembers(); ++j) {
            node->addMember(sub->getMemberUnchecked(j));
          }
          node->changeMember(i, _ast->createNodeNop());
        }
        else { 
          // different type
          node->changeMember(i, sub);
        }
      }
    }

    return node;
  };

  std::function<AstNode*(AstNode*, int)> reroot = [this, &reroot] (AstNode* node, int level) -> AstNode* {
    if (node == nullptr) {
      return nullptr;
    }

    AstNodeType type;

    if (level == 0) {
      type = NODE_TYPE_OPERATOR_NARY_OR;
    }
    else {
      type = NODE_TYPE_OPERATOR_NARY_AND;
    }
    // check if first-level node is an OR node
    if (node->type != type) {
      // create new root node
      node = _ast->createNodeNaryOperator(type, node);
    }

    size_t const n = node->numMembers();
    size_t j = 0;
    for (size_t i = 0; i < n; ++i) {
      auto sub = node->getMemberUnchecked(i);

      if (sub->type == NODE_TYPE_NOP) {
        // ignore this node
        continue;
      }

      if (level == 0) {
        // recurse into next level
        node->changeMember(j, reroot(sub, level + 1));
      }
      else if (i != j) {
        node->changeMember(j, sub);
      }
      ++j;
    }

    if (j != n) {
      // adjust number of members (because of the NOP nodes removes)
      node->reduceMembers(j);
    }

    return node;
  };

//std::cout << "\n";
  _root = transform(_root);
  _root = collapse(_root);
  _root = reroot(_root, 0);

//dump();
//std::cout << "\n";
}

////////////////////////////////////////////////////////////////////////////////
/// @brief dump a condition
////////////////////////////////////////////////////////////////////////////////

void Condition::dump () const {
  std::function<void(AstNode const*, int)> dumpNode;

  dumpNode = [&dumpNode] (AstNode const* node, int indent) {
    if (node == nullptr) {
      return;
    }

    for (int i = 0; i < indent * 2; ++i) {
      std::cout << " ";
    }

    std::cout << node->getTypeString();
    if (node->type == NODE_TYPE_VALUE) {
      std::cout << "  (value " << triagens::basics::JsonHelper::toString(node->toJsonValue(TRI_UNKNOWN_MEM_ZONE)) << ")";
    }
    else if (node->type == NODE_TYPE_ATTRIBUTE_ACCESS) {
      std::cout << "  (attribute " << node->getStringValue() << ")";
    }

    std::cout << "\n";
    for (size_t i = 0; i < node->numMembers(); ++i) {
      dumpNode(node->getMember(i), indent + 1);
    }
  };

  dumpNode(_root, 0);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
