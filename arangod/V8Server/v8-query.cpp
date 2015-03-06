////////////////////////////////////////////////////////////////////////////////
/// @brief V8-vocbase queries
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
/// @author Dr. Frank Celler
/// @author Copyright 2014, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "v8-query.h"

#include "Basics/string-buffer.h"
#include "Mvcc/CollectionOperations.h"
#include "Mvcc/EdgeIndex.h"
#include "Mvcc/FulltextIndex.h"
#include "Mvcc/GeoIndex.h"
#include "Mvcc/HashIndex.h"
#include "Mvcc/MasterpointerManager.h"
#include "Mvcc/ScopedResolver.h"
#include "Mvcc/SkiplistIndex.h"
#include "Mvcc/Transaction.h"
#include "Mvcc/TransactionCollection.h"
#include "Mvcc/TransactionScope.h"
#include "Utils/CollectionNameResolver.h"
#include "V8/v8-globals.h"
#include "V8/v8-conv.h"
#include "V8/v8-utils.h"
#include "V8Server/v8-vocbase.h"
#include "V8Server/v8-vocindex.h"
#include "V8Server/v8-wrapshapedjson.h"
#include "VocBase/document-collection.h"
#include "VocBase/vocbase.h"
#include "VocBase/voc-shaper.h"

using namespace std;
using namespace triagens::arango;

////////////////////////////////////////////////////////////////////////////////
/// @brief query types
////////////////////////////////////////////////////////////////////////////////

typedef enum {
  QUERY_EXAMPLE,
  QUERY_CONDITION
}
QueryType;

// -----------------------------------------------------------------------------
// --SECTION--                                                  helper functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief extracts skip and limit
////////////////////////////////////////////////////////////////////////////////

static void ExtractSkipAndLimit (const v8::FunctionCallbackInfo<v8::Value>& args,
                                 size_t pos,
                                 TRI_voc_ssize_t& skip,
                                 TRI_voc_size_t& limit) {

  skip = TRI_QRY_NO_SKIP;
  limit = TRI_QRY_NO_LIMIT;

  if (pos < (size_t) args.Length() && ! args[(int) pos]->IsNull() && ! args[(int) pos]->IsUndefined()) {
    skip = (TRI_voc_ssize_t) TRI_ObjectToDouble(args[(int) pos]);
  }

  if (pos + 1 < (size_t) args.Length() && ! args[(int) pos + 1]->IsNull() && ! args[(int) pos + 1]->IsUndefined()) {
    limit = (TRI_voc_size_t) TRI_ObjectToDouble(args[(int) pos + 1]);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief calculates slice
////////////////////////////////////////////////////////////////////////////////

static void CalculateSkipLimitSlice (size_t length,
                                     TRI_voc_ssize_t skip,
                                     TRI_voc_size_t limit,
                                     size_t& s,
                                     size_t& e) {
  s = 0;
  e = length;

  // skip from the beginning
  if (0 < skip) {
    s = (size_t) skip;

    if (e < s) {
      s = (size_t) e;
    }
  }

  // skip from the end
  else if (skip < 0) {
    skip = -skip;

    if ((size_t) skip < e) {
      s = e - skip;
    }
  }

  // apply limit
  if (s + limit < e) {
    int64_t sum = (int64_t) s + (int64_t) limit;
    if (sum < (int64_t) e) {
      if (sum >= (int64_t) TRI_QRY_NO_LIMIT) {
        e = TRI_QRY_NO_LIMIT;
      }
      else {
        e = (size_t) sum;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief fetches all documents from a collection
////////////////////////////////////////////////////////////////////////////////

static void JS_MvccAll (const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope scope(isolate);

  auto const* collection = TRI_UnwrapClass<TRI_vocbase_col_t const>(args.Holder(), TRI_GetVocBaseColType());

  if (collection == nullptr) {
    TRI_V8_THROW_EXCEPTION_INTERNAL("cannot extract collection");
  }
  
  uint32_t const argLength = args.Length();
  if (argLength > 2) {
    TRI_V8_THROW_EXCEPTION_USAGE("all(<skip>, <limit>)");
  }
  
  triagens::mvcc::SearchOptions searchOptions;
  if (args.Length() > 0 && ! args[0]->IsNull() && ! args[0]->IsUndefined()) {
    searchOptions.skip = TRI_ObjectToInt64(args[0]);
  }

  if (args.Length() > 1 && ! args[1]->IsNull() && ! args[1]->IsUndefined()) {
    searchOptions.limit = TRI_ObjectToInt64(args[1]);
  }

  triagens::mvcc::OperationOptions options;
  options.searchOptions = &searchOptions;

  // need a fake old transaction in order to not throw - can be removed later       
  TransactionBase oldTrx(true);
  triagens::mvcc::ScopedResolver resolver(collection->_vocbase);

  try {
    triagens::mvcc::TransactionScope transactionScope(collection->_vocbase, triagens::mvcc::TransactionScope::NoCollections());

    auto* transaction = transactionScope.transaction();
    transaction->resolver(resolver.get()); // inject resolver into other transaction
    auto* transactionCollection = transaction->collection(collection->_cid);

    std::vector<TRI_doc_mptr_t const*> foundDocuments;
    auto searchResult = triagens::mvcc::CollectionOperations::ReadAllDocuments(&transactionScope, transactionCollection, foundDocuments, options);
 
    if (searchResult.code != TRI_ERROR_NO_ERROR) {
      THROW_ARANGO_EXCEPTION(searchResult.code);
    }
  
    // setup result
    size_t const n = foundDocuments.size();
    uint32_t count = 0;
    uint32_t total = 0;

    v8::Handle<v8::Object> result = v8::Object::New(isolate);
    v8::Handle<v8::Array> documents = v8::Array::New(isolate, static_cast<int>(n));
  
    for (size_t i = 0; i < n; ++i) {
      v8::Handle<v8::Value> document = TRI_WrapShapedJson(isolate, transaction->resolver(), transactionCollection, foundDocuments[i]->getDataPtr());

      if (document.IsEmpty()) {
        TRI_V8_THROW_EXCEPTION_MEMORY();
      }
      else {
        documents->Set(count++, document);
      }
    }

    result->ForceSet(TRI_V8_ASCII_STRING("documents"), documents);
    result->ForceSet(TRI_V8_ASCII_STRING("total"), v8::Number::New(isolate, total));
    result->ForceSet(TRI_V8_ASCII_STRING("count"), v8::Number::New(isolate, count));

    TRI_V8_RETURN(result);
  }
  catch (triagens::arango::Exception const& ex) {
    TRI_V8_THROW_EXCEPTION_MESSAGE(ex.code(), ex.what());
  }
  catch (...) {
    TRI_V8_THROW_EXCEPTION(TRI_ERROR_INTERNAL);
  }
 
  // unreachable
  TRI_ASSERT(false);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                     random access
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief selects a random document
///
/// @FUN{@FA{collection}.any()}
///
/// The @FN{any} method returns a random document from the collection.  It returns
/// @LIT{null} if the collection is empty.
///
/// @EXAMPLES
///
/// @code
/// arangod> db.example.any()
/// { "_id" : "example/222716379559", "_rev" : "222716379559", "Hello" : "World" }
/// @endcode
////////////////////////////////////////////////////////////////////////////////

static void JS_MvccAny (const v8::FunctionCallbackInfo<v8::Value>& args) {
  TransactionBase transBase(true);   // To protect against assertions, FIXME later
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope scope(isolate);

  auto const* collection = TRI_UnwrapClass<TRI_vocbase_col_t>(args.Holder(), TRI_GetVocBaseColType());

  if (collection == nullptr) {
    TRI_V8_THROW_EXCEPTION_INTERNAL("cannot extract collection");
  }
    
  try {
    triagens::mvcc::TransactionScope transactionScope(collection->_vocbase, triagens::mvcc::TransactionScope::NoCollections());

    auto* transaction = transactionScope.transaction();
    auto* transactionCollection = transaction->collection(collection->_cid);
  
    auto readResult = triagens::mvcc::CollectionOperations::RandomDocument(&transactionScope, transactionCollection);
    
    if (readResult.code != TRI_ERROR_NO_ERROR) {
      THROW_ARANGO_EXCEPTION(readResult.code);
    }

    if (readResult.mptr == nullptr) {
      // collection is empty
      TRI_V8_RETURN_NULL();
    }

    // convert to v8
    v8::Handle<v8::Value> result = TRI_WrapShapedJson(isolate, transaction->resolver(), transactionCollection, readResult.mptr->getDataPtr());

    if (result.IsEmpty()) {
      TRI_V8_THROW_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
    }

    TRI_V8_RETURN(result);
  }
  catch (triagens::arango::Exception const& ex) {
    TRI_V8_THROW_EXCEPTION(ex.code());
  }
  catch (...) {
    TRI_V8_THROW_EXCEPTION(TRI_ERROR_INTERNAL);
  }

  TRI_ASSERT(false);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  by example query
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief destroys the example object
////////////////////////////////////////////////////////////////////////////////

static void DestroyExample (std::vector<TRI_shaped_json_t*>& values) {
  for (auto it : values) {
    if (it != nullptr) {
      TRI_FreeShapedJson(TRI_UNKNOWN_MEM_ZONE, it);
    }
  }

  values.clear();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief sets up the example object
////////////////////////////////////////////////////////////////////////////////

static int SetupExample (v8::Isolate* isolate,
                         std::vector<TRI_shape_pid_t>& pids,
                         std::vector<TRI_shaped_json_t*>& values,
                         v8::Handle<v8::Object> const example,
                         TRI_shaper_t* shaper) {

  // get own properties of example
  v8::Handle<v8::Array> names = example->GetOwnPropertyNames();
  size_t n = names->Length();

  pids.reserve(n);
  values.reserve(n);

  // convert
  for (size_t i = 0;  i < n;  ++i) {
    v8::Handle<v8::Value> key = names->Get((uint32_t) i);
    v8::Handle<v8::Value> val = example->Get(key);

    // property initialise the memory
    values[i] = 0;

    TRI_Utf8ValueNFC keyStr(TRI_UNKNOWN_MEM_ZONE, key);

    if (*keyStr == nullptr) {
      return TRI_ERROR_BAD_PARAMETER;
    }

    auto pid = shaper->lookupAttributePathByName(shaper, *keyStr);

    if (pid == 0) {
      // no attribute path found. this means the result will be empty
      return TRI_RESULT_ELEMENT_NOT_FOUND;
    }

    pids.push_back(pid);

    auto shaped = TRI_ShapedJsonV8Object(isolate, val, shaper, false);

    if (shaped == nullptr) {
      return TRI_RESULT_ELEMENT_NOT_FOUND;
    }

    try {
      values.push_back(shaped);
    }
    catch (...) {
      TRI_FreeShapedJson(shaper->_memoryZone, shaped);
      throw;
    }
  }

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief selects documents by example, full collection scan
////////////////////////////////////////////////////////////////////////////////

static void JS_MvccByExample (const v8::FunctionCallbackInfo<v8::Value>& args) {
  TransactionBase transBase(true);   // To protect against assertions, FIXME later
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope scope(isolate);

  auto const* collection = TRI_UnwrapClass<TRI_vocbase_col_t>(args.Holder(), TRI_GetVocBaseColType());

  if (collection == nullptr) {
    TRI_V8_THROW_EXCEPTION_INTERNAL("cannot extract collection");
  }

  TRI_THROW_SHARDING_COLLECTION_NOT_YET_IMPLEMENTED(collection);

  // expecting index, example, skip, and limit
  if (args.Length() < 1) {
    TRI_V8_THROW_EXCEPTION_USAGE("byExample(<example>, <skip>, <limit>)");
  }

  // extract the example
  if (! args[0]->IsObject()) {
    TRI_V8_THROW_TYPE_ERROR("<example> must be an object");
  }

  v8::Handle<v8::Object> example = args[0]->ToObject();

  // extract skip and limit
  TRI_voc_ssize_t skip;
  TRI_voc_size_t limit;
  ExtractSkipAndLimit(args, 1, skip, limit);
  
  triagens::mvcc::SearchOptions searchOptions;
  searchOptions.skip    = skip;
  searchOptions.limit   = limit;
  
  triagens::mvcc::OperationOptions options;
  options.searchOptions = &searchOptions;
    
  try {
    triagens::mvcc::TransactionScope transactionScope(collection->_vocbase, triagens::mvcc::TransactionScope::NoCollections());

    auto* transaction = transactionScope.transaction();
    auto* transactionCollection = transaction->collection(collection->_cid);
    auto resolver = transaction->resolver();
  
    std::vector<TRI_shape_pid_t> pids;
    std::vector<TRI_shaped_json_t*> values;

    int res = SetupExample(isolate, pids, values, example, transactionCollection->shaper());

    if (res != TRI_ERROR_NO_ERROR) {
      DestroyExample(values);

      if (res == TRI_RESULT_ELEMENT_NOT_FOUND) {
        v8::Handle<v8::Object> result = v8::Object::New(isolate);
        result->Set(TRI_V8_ASCII_STRING("documents"), v8::Array::New(isolate));
        result->Set(TRI_V8_ASCII_STRING("total"), v8::Integer::New(isolate, 0));
        result->Set(TRI_V8_ASCII_STRING("count"), v8::Integer::New(isolate, 0));

        TRI_V8_RETURN(result);
      }
      TRI_V8_THROW_EXCEPTION(res);
    }

    std::vector<TRI_doc_mptr_t const*> foundDocuments;
    auto searchResult = triagens::mvcc::CollectionOperations::ReadByExample(&transactionScope, transactionCollection, pids, values, foundDocuments, options);
    DestroyExample(values);
 
    if (searchResult.code != TRI_ERROR_NO_ERROR) {
      THROW_ARANGO_EXCEPTION(searchResult.code);
    }
      
    size_t const n = foundDocuments.size();
    v8::Handle<v8::Object> result = v8::Object::New(isolate);
    auto documents = v8::Array::New(isolate, static_cast<int>(n));

    for (size_t i = 0; i < n; ++i) {
      v8::Handle<v8::Value> document = TRI_WrapShapedJson(isolate, resolver, transactionCollection, foundDocuments[i]->getDataPtr());

      if (document.IsEmpty()) {
        TRI_V8_THROW_EXCEPTION_MEMORY();
      }
      documents->Set(static_cast<uint32_t>(i), document);
    }
  
    result->Set(TRI_V8_ASCII_STRING("documents"), documents);
    result->Set(TRI_V8_ASCII_STRING("total"), v8::Integer::New(isolate, static_cast<int>(n)));
    result->Set(TRI_V8_ASCII_STRING("count"), v8::Integer::New(isolate, static_cast<int>(n)));

    TRI_V8_RETURN(result);
  }
  catch (triagens::arango::Exception const& ex) {
    TRI_V8_THROW_EXCEPTION(ex.code());
  }
  catch (...) {
    TRI_V8_THROW_EXCEPTION(TRI_ERROR_INTERNAL);
  }

  TRI_ASSERT(false);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  hash index query
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief destroys the example object for a hash index query
////////////////////////////////////////////////////////////////////////////////

static void DestroySearchValue (std::vector<TRI_shaped_json_t>& searchValue) {
  for (auto& it : searchValue) {
    TRI_DestroyShapedJson(TRI_UNKNOWN_MEM_ZONE, &it);
  }
  searchValue.clear();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief sets up the example object for a hash index query
////////////////////////////////////////////////////////////////////////////////

static int SetupSearchValue (v8::Isolate* isolate,
                             std::vector<TRI_shape_pid_t> const& paths,
                             v8::Handle<v8::Object> const example,
                             TRI_shaper_t* shaper,
                             std::vector<TRI_shaped_json_t>& result) {
  TRI_ASSERT(result.size() == paths.size());
  TRI_ASSERT(! paths.empty());

  for (size_t i = 0; i < paths.size(); ++i) {
    auto pid = paths[i];
    TRI_ASSERT(pid != 0);
    char const* name = TRI_AttributeNameShapePid(shaper, pid);

    if (name == nullptr) {
      return TRI_ERROR_BAD_PARAMETER;
    }

    v8::Handle<v8::String> key = TRI_V8_STRING(name);
    int res;

    if (example->HasOwnProperty(key)) {
      res = TRI_FillShapedJsonV8Object(isolate, example->Get(key), &result[i], shaper, false);
    }
    else {
      res = TRI_FillShapedJsonV8Object(isolate, v8::Null(isolate), &result[i], shaper, false);
    }

    if (res != TRI_ERROR_NO_ERROR) {
      return res;
    }
  }

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief selects documents by example using a hash index
////////////////////////////////////////////////////////////////////////////////

static void JS_MvccByExampleHash (const v8::FunctionCallbackInfo<v8::Value>& args) {
  TransactionBase transBase(true);   // To protect against assertions, FIXME later
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope scope(isolate);

  auto const* collection = TRI_UnwrapClass<TRI_vocbase_col_t>(args.Holder(), TRI_GetVocBaseColType());

  if (collection == nullptr) {
    TRI_V8_THROW_EXCEPTION_INTERNAL("cannot extract collection");
  }

  TRI_THROW_SHARDING_COLLECTION_NOT_YET_IMPLEMENTED(collection);

  // expecting index, example, skip, and limit
  if (args.Length() < 2) {
    TRI_V8_THROW_EXCEPTION_USAGE("byExampleHash(<index>, <example>, <skip>, <limit>)");
  }

  // extract the example
  if (! args[1]->IsObject()) {
    TRI_V8_THROW_TYPE_ERROR("<example> must be an object");
  }

  v8::Handle<v8::Object> example = args[1]->ToObject();

  // extract skip and limit
  TRI_voc_ssize_t skip;
  TRI_voc_size_t limit;
  ExtractSkipAndLimit(args, 2, skip, limit);

  try {
    triagens::mvcc::TransactionScope transactionScope(collection->_vocbase, triagens::mvcc::TransactionScope::NoCollections());

    auto* transaction = transactionScope.transaction();
    auto* transactionCollection = transaction->collection(collection->_cid);
    auto resolver = transaction->resolver();
  
    // extract the index, and hold a read-lock on the list of indexes while we're using the index
    triagens::mvcc::IndexUser indexUser(transactionCollection);
    auto index = TRI_LookupMvccIndexByHandle(isolate, resolver, collection, args[0]);

    if (index == nullptr ||
        index->type() != TRI_IDX_TYPE_HASH_INDEX) {
      TRI_V8_THROW_EXCEPTION(TRI_ERROR_ARANGO_NO_INDEX);
    }

    auto hashIndex = static_cast<triagens::mvcc::HashIndex*>(index);
    auto const& paths = hashIndex->paths();

    std::vector<TRI_shaped_json_t> searchValue(paths.size());

    int res = SetupSearchValue(isolate, paths, example, transactionCollection->shaper(), searchValue);

    if (res != TRI_ERROR_NO_ERROR) {
      DestroySearchValue(searchValue);

      if (res == TRI_RESULT_ELEMENT_NOT_FOUND) {
        TRI_V8_RETURN(v8::Array::New(isolate));
      }
      TRI_V8_THROW_EXCEPTION(res);
    }

    // setup result
    std::unique_ptr<std::vector<TRI_doc_mptr_t*>> indexResult(hashIndex->lookup(transactionCollection, transaction, &searchValue, 0)); 
    DestroySearchValue(searchValue);
 
    if (indexResult.get() == nullptr) {
      TRI_V8_RETURN(v8::Array::New(isolate));
    }

    auto const& foundDocuments = *(indexResult.get());

    size_t total = indexResult->size();
    auto result = v8::Object::New(isolate);
    auto documents = v8::Array::New(isolate);

    if (total > 0) {
      size_t s;
      size_t e;
      CalculateSkipLimitSlice(total, skip, limit, s, e);

      if (s < e) {
        size_t count = 0;

        for (size_t i = s;  i < e;  ++i) {
          v8::Handle<v8::Value> document = TRI_WrapShapedJson(isolate, resolver, transactionCollection, foundDocuments[i]->getDataPtr());

          if (document.IsEmpty()) {
            TRI_V8_THROW_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
            break;
          }

          documents->Set(static_cast<uint32_t>(count++), document);
        }
      }
    }
    
    result->Set(TRI_V8_ASCII_STRING("documents"), documents);
    result->Set(TRI_V8_ASCII_STRING("total"), v8::Integer::New(isolate, static_cast<int>(documents->Length())));
    result->Set(TRI_V8_ASCII_STRING("count"), v8::Integer::New(isolate, static_cast<int>(documents->Length())));
  
    TRI_V8_RETURN(result);
  }
  catch (triagens::arango::Exception const& ex) {
    TRI_V8_THROW_EXCEPTION(ex.code());
  }
  catch (...) {
    TRI_V8_THROW_EXCEPTION(TRI_ERROR_INTERNAL);
  }

  TRI_ASSERT(false);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  skiplist queries
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief sets up the skiplist operator for a skiplist condition query
////////////////////////////////////////////////////////////////////////////////

static TRI_index_operator_t* SetupSkiplistCondition (v8::Isolate* isolate,
                                                     std::vector<TRI_shape_pid_t> const& paths,
                                                     v8::Handle<v8::Object> const example,
                                                     TRI_shaper_t* shaper) {
  TRI_json_t* parameters = TRI_CreateArrayJson(TRI_UNKNOWN_MEM_ZONE);

  if (parameters == nullptr) {
    return nullptr;
  }
  
  TRI_index_operator_t* lastOperator = nullptr;

  // free function, called when anything goes wrong 
  auto freeOperator = [&] () -> void {
    TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, parameters);

    if (lastOperator != nullptr) {
      TRI_FreeIndexOperator(lastOperator);
    }
  };

  size_t numEq = 0;
  size_t lastNonEq = 0;

  // iterate over all index fields
  for (size_t i = 1; i <= paths.size(); ++i) {
    auto pid = paths[i - 1];
    TRI_ASSERT(pid != 0);
    char const* name = TRI_AttributeNameShapePid(shaper, pid);
    
    v8::Handle<v8::String> key = TRI_V8_STRING(name);

    if (! example->HasOwnProperty(key)) {
      break;
    }

    v8::Handle<v8::Value> fieldConditions = example->Get(key);

    if (! fieldConditions->IsArray()) {
      // wrong data type for field conditions
      break;
    }

    // iterator over all conditions
    v8::Handle<v8::Array> values = v8::Handle<v8::Array>::Cast(fieldConditions);

    for (uint32_t j = 0; j < values->Length(); ++j) {
      v8::Handle<v8::Value> fieldCondition = values->Get(j);

      if (! fieldCondition->IsArray() ||
          v8::Handle<v8::Array>::Cast(fieldCondition)->Length() != 2) {
        // wrong data type for single condition
        freeOperator();
        return nullptr;
      }

      v8::Handle<v8::Array> condition = v8::Handle<v8::Array>::Cast(fieldCondition);
      v8::Handle<v8::Value> op = condition->Get(0);
      v8::Handle<v8::Value> value = condition->Get(1);

      if (! op->IsString() && ! op->IsStringObject()) {
        // wrong operator type
        freeOperator();
        return nullptr;
      }

      std::unique_ptr<TRI_json_t> json(TRI_ObjectToJson(isolate, value));

      if (json.get() == nullptr) {
        freeOperator();
        return nullptr;
      }

      std::string&& opValue = TRI_ObjectToString(op);
      if (opValue == "==") {
        // equality comparison

        if (lastNonEq > 0) {
          freeOperator();
          return nullptr;
        }

        TRI_PushBack3ArrayJson(TRI_UNKNOWN_MEM_ZONE, parameters, json.release());
        // creation of equality operator is deferred until it is finally needed
        ++numEq;
        break;
      }


      if (lastNonEq > 0 && lastNonEq != i) {
        // if we already had a range condition and a previous field, we cannot continue
        // because the skiplist interface does not support such queries
        freeOperator();
        return nullptr;
      }

      TRI_index_operator_type_e opType;
      if (opValue == ">") {
        opType = TRI_GT_INDEX_OPERATOR;
      }
      else if (opValue == ">=") {
        opType = TRI_GE_INDEX_OPERATOR;
      }
      else if (opValue == "<") {
        opType = TRI_LT_INDEX_OPERATOR;
      }
      else if (opValue == "<=") {
        opType = TRI_LE_INDEX_OPERATOR;
      }
      else {
        // wrong operator type
        freeOperator();
        return nullptr;
      }

      lastNonEq = i;

      TRI_json_t* cloned = TRI_CopyJson(TRI_UNKNOWN_MEM_ZONE, parameters);

      if (cloned == nullptr) {
        freeOperator();
        return nullptr;
      }

      TRI_PushBack3ArrayJson(TRI_UNKNOWN_MEM_ZONE, cloned, json.release());

      if (numEq) {
        // create equality operator if one is in queue
        TRI_json_t* clonedParams = TRI_CopyJson(TRI_UNKNOWN_MEM_ZONE, parameters);

        if (clonedParams == nullptr) {
          TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, cloned);
          freeOperator();
          return nullptr;
        }

        lastOperator = TRI_CreateIndexOperator(TRI_EQ_INDEX_OPERATOR, 
                                               nullptr,
                                               nullptr, 
                                               clonedParams, 
                                               shaper, 
                                               TRI_LengthArrayJson(clonedParams));
        numEq = 0;
      }

      // create the operator for the current condition
      TRI_index_operator_t* current = TRI_CreateIndexOperator(opType,
                                                              nullptr,
                                                              nullptr, 
                                                              cloned, 
                                                              shaper, 
                                                              TRI_LengthArrayJson(cloned));

      if (current == nullptr) {
        TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, cloned);
        freeOperator();
        return nullptr;
      }

      if (lastOperator == nullptr) {
        lastOperator = current;
      }
      else {
        // merge the current operator with previous operators using logical AND
        TRI_index_operator_t* newOperator = TRI_CreateIndexOperator(TRI_AND_INDEX_OPERATOR, 
                                                                    lastOperator, 
                                                                    current, 
                                                                    nullptr, 
                                                                    shaper, 
                                                                    2);

        if (newOperator == nullptr) {
          TRI_FreeIndexOperator(current);
          freeOperator();
          return nullptr;
        }
        else {
          lastOperator = newOperator;
        }
      }
    }

  }

  if (numEq) {
    // create equality operator if one is in queue
    TRI_ASSERT(lastOperator == nullptr);
    TRI_ASSERT(lastNonEq == 0);

    TRI_json_t* clonedParams = TRI_CopyJson(TRI_UNKNOWN_MEM_ZONE, parameters);

    if (clonedParams == nullptr) {
      freeOperator();
      return nullptr;
    }

    lastOperator = TRI_CreateIndexOperator(TRI_EQ_INDEX_OPERATOR, 
                                           nullptr,
                                           nullptr,
                                           clonedParams, 
                                           shaper, 
                                           TRI_LengthArrayJson(clonedParams));
  }

  TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, parameters);

  return lastOperator;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief sets up the skiplist operator for a skiplist example query
///
/// this will set up a JSON container with the example values as a list
/// at the end, one skiplist equality operator is created for the entire list
////////////////////////////////////////////////////////////////////////////////

static TRI_index_operator_t* SetupSkiplistExample (v8::Isolate* isolate,
                                                   std::vector<TRI_shape_pid_t> const& paths,
                                                   v8::Handle<v8::Object> const example,
                                                   TRI_shaper_t* shaper) {
  std::unique_ptr<TRI_json_t> parameters(TRI_CreateArrayJson(TRI_UNKNOWN_MEM_ZONE));

  if (parameters.get() == nullptr) {
    return nullptr;
  }
  
  TRI_ASSERT(! paths.empty());

  for (size_t i = 0; i < paths.size(); ++i) {
    auto pid = paths[i];
    TRI_ASSERT(pid != 0);
    char const* name = TRI_AttributeNameShapePid(shaper, pid);
    
    v8::Handle<v8::String> key = TRI_V8_STRING(name);

    if (! example->HasOwnProperty(key)) {
      break;
    }

    v8::Handle<v8::Value> value = example->Get(key);

    TRI_json_t* json = TRI_ObjectToJson(isolate, value);

    if (json == nullptr) {
      return nullptr;
    }

    TRI_PushBack3ArrayJson(TRI_UNKNOWN_MEM_ZONE, parameters.get(), json);
  }

  size_t const n = TRI_LengthArrayJson(parameters.get());

  if (n == 0) {
    // JSON is empty. now create an operator for the full range
    TRI_PushBack3ArrayJson(TRI_UNKNOWN_MEM_ZONE, parameters.get(), TRI_CreateNullJson(TRI_UNKNOWN_MEM_ZONE));

    return TRI_CreateIndexOperator(TRI_GE_INDEX_OPERATOR, 
                                   nullptr,
                                   nullptr, 
                                   parameters.release(), 
                                   shaper, 
                                   1);
  }
  
  // example means equality comparisons only
  return TRI_CreateIndexOperator(TRI_EQ_INDEX_OPERATOR, 
                                 nullptr, 
                                 nullptr,
                                 parameters.release(), 
                                 shaper, 
                                 n);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief selects documents by example using a hash index
////////////////////////////////////////////////////////////////////////////////

static void MvccSkiplistQuery (QueryType type,
                               const v8::FunctionCallbackInfo<v8::Value>& args) {
  TransactionBase transBase(true);   // To protect against assertions, FIXME later
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope scope(isolate);

  auto const* collection = TRI_UnwrapClass<TRI_vocbase_col_t>(args.Holder(), TRI_GetVocBaseColType());

  if (collection == nullptr) {
    TRI_V8_THROW_EXCEPTION_INTERNAL("cannot extract collection");
  }

  TRI_THROW_SHARDING_COLLECTION_NOT_YET_IMPLEMENTED(collection);

  // expecting index, example, skip, and limit
  if (args.Length() < 2) {
    if (type == QUERY_CONDITION) {
      TRI_V8_THROW_EXCEPTION_USAGE("byConditionSkiplist(<index>, <conditions>, <skip>, <limit>, <reverse>)");
    }
    TRI_V8_THROW_EXCEPTION_USAGE("byExampleSkiplist(<index>, <example>, <skip>, <limit>, <reverse>)");
  }
  
  // extract the example
  if (! args[1]->IsObject()) {
    if (type == QUERY_CONDITION) {
      TRI_V8_THROW_TYPE_ERROR("<conditions> must be an object");
    }
    TRI_V8_THROW_TYPE_ERROR("<example> must be an object");
  }

  bool reverse = false;
  if (args.Length() > 4) {
    reverse = TRI_ObjectToBoolean(args[4]);
  }


  // extract skip and limit
  TRI_voc_ssize_t skip;
  TRI_voc_size_t limit;
  ExtractSkipAndLimit(args, 2, skip, limit);

  try {
    triagens::mvcc::TransactionScope transactionScope(collection->_vocbase, triagens::mvcc::TransactionScope::NoCollections());

    auto* transaction = transactionScope.transaction();
    auto* transactionCollection = transaction->collection(collection->_cid);
    auto resolver = transaction->resolver();
  
    // extract the index, and hold a read-lock on the list of indexes while we're using the index
    triagens::mvcc::IndexUser indexUser(transactionCollection);
    auto index = TRI_LookupMvccIndexByHandle(isolate, resolver, collection, args[0]);

    if (index == nullptr ||
        index->type() != TRI_IDX_TYPE_SKIPLIST_INDEX) {
      TRI_V8_THROW_EXCEPTION(TRI_ERROR_ARANGO_NO_INDEX);
    }

    auto skiplistIndex = static_cast<triagens::mvcc::SkiplistIndex*>(index);
    auto const& paths = skiplistIndex->paths();

    auto example = args[1]->ToObject();

    std::unique_ptr<TRI_index_operator_t> skiplistOperator;
    if (type == QUERY_CONDITION) {
      skiplistOperator.reset(SetupSkiplistCondition(isolate, paths, example, transactionCollection->shaper()));
    }
    else {
      skiplistOperator.reset(SetupSkiplistExample(isolate, paths, example, transactionCollection->shaper()));
    }

    if (skiplistOperator.get() == nullptr) {
      TRI_V8_THROW_EXCEPTION(TRI_ERROR_BAD_PARAMETER);
    }

    // setup result
    std::unique_ptr<triagens::mvcc::SkiplistIndex::Iterator> iter(skiplistIndex->lookup(transactionCollection, transaction, skiplistOperator.get(), reverse));

    std::unique_ptr<std::vector<TRI_doc_mptr_t*>> indexResult
        (new std::vector<TRI_doc_mptr_t*>);
    
    iter->skip(skip);
    size_t count = 0;
    while (count < limit && iter->hasNext()) {
      triagens::mvcc::SkiplistIndex::Element* elm = iter->next();
      if (elm != nullptr) {   // MVCC logic might actually not see something
                              // that hasNext was suspecting to be a result!
        TRI_doc_mptr_t* ptr = elm->_document;
        indexResult->emplace_back(ptr);
        count++;
      }
    }
 
    auto const& foundDocuments = *(indexResult.get());

    auto result = v8::Object::New(isolate);
    auto documents = v8::Array::New(isolate, foundDocuments.size());
    uint32_t i = 0;

    for (auto const& it : foundDocuments) {
      v8::Handle<v8::Value> document = TRI_WrapShapedJson(isolate, resolver, transactionCollection, it->getDataPtr());

      if (document.IsEmpty()) {
        TRI_V8_THROW_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
      }

      documents->Set(i++, document);
    }
    
    result->Set(TRI_V8_ASCII_STRING("documents"), documents);
    result->Set(TRI_V8_ASCII_STRING("total"), v8::Integer::New(isolate, static_cast<int>(documents->Length())));
    result->Set(TRI_V8_ASCII_STRING("count"), v8::Integer::New(isolate, static_cast<int>(documents->Length())));

    TRI_V8_RETURN(result);
  }
  catch (triagens::arango::Exception const& ex) {
    TRI_V8_THROW_EXCEPTION(ex.code());
  }
  catch (...) {
    TRI_V8_THROW_EXCEPTION(TRI_ERROR_INTERNAL);
  }

  TRI_ASSERT(false);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief query a skiplist by condition
////////////////////////////////////////////////////////////////////////////////

static void JS_MvccByConditionSkiplist (const v8::FunctionCallbackInfo<v8::Value>& args) {
  MvccSkiplistQuery(QUERY_CONDITION, args);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief query a skiplist by example
////////////////////////////////////////////////////////////////////////////////

static void JS_MvccByExampleSkiplist (const v8::FunctionCallbackInfo<v8::Value>& args) {
  MvccSkiplistQuery(QUERY_EXAMPLE, args);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                    checksum query
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief helper struct used when calculting checksums
////////////////////////////////////////////////////////////////////////////////

struct CollectionChecksumHelper {
  CollectionChecksumHelper (CollectionNameResolver const* resolver,
                            TRI_shaper_t* shaper,
                            bool withRevisions,
                            bool withData)
    : resolver(resolver),
      shaper(shaper),
      checksum(0),
      withRevisions(withRevisions),
      withData(withData) {
      
    TRI_InitStringBuffer(&buffer, TRI_UNKNOWN_MEM_ZONE);
  }
  
  ~CollectionChecksumHelper () {
    TRI_DestroyStringBuffer(&buffer);
  }

  void update (TRI_doc_mptr_t const* mptr) {
    TRI_df_marker_t const* marker = static_cast<TRI_df_marker_t const*>(mptr->getDataPtr());  
    uint32_t localCrc;

    if (marker->_type == TRI_DOC_MARKER_KEY_DOCUMENT ||
        marker->_type == TRI_WAL_MARKER_DOCUMENT ||
        marker->_type == TRI_WAL_MARKER_MVCC_DOCUMENT) {
      localCrc = TRI_Crc32HashString(TRI_EXTRACT_MARKER_KEY(mptr));  
      if (withRevisions) {
        localCrc += TRI_Crc32HashPointer(&mptr->_rid, sizeof(TRI_voc_rid_t));
      }
    }
    else if (marker->_type == TRI_DOC_MARKER_KEY_EDGE ||
             marker->_type == TRI_WAL_MARKER_EDGE ||
             marker->_type == TRI_WAL_MARKER_MVCC_EDGE) {
      TRI_voc_cid_t fromCid;
      TRI_voc_cid_t toCid;
      uint32_t offsetFromKey;
      uint32_t offsetToKey;

      // must convert _rid, _fromCid, _toCid into strings for portability
      localCrc = TRI_Crc32HashString(TRI_EXTRACT_MARKER_KEY(mptr)); 
      if (withRevisions) {
        localCrc += TRI_Crc32HashPointer(&mptr->_rid, sizeof(TRI_voc_rid_t));
      }

      if (marker->_type == TRI_DOC_MARKER_KEY_EDGE) {
        auto* e = reinterpret_cast<TRI_doc_edge_key_marker_t const*>(marker);
        fromCid       = e->_fromCid;
        toCid         = e->_toCid;
        offsetFromKey = e->_offsetFromKey;
        offsetToKey   = e->_offsetToKey;
      }
      else if (marker->_type == TRI_WAL_MARKER_EDGE) {
        auto* e = reinterpret_cast<triagens::wal::edge_marker_t const*>(marker);
        fromCid       = e->_fromCid;
        toCid         = e->_toCid;
        offsetFromKey = e->_offsetFromKey;
        offsetToKey   = e->_offsetToKey;
      }
      else {
        auto* e = reinterpret_cast<triagens::wal::mvcc_edge_marker_t const*>(marker);
        fromCid       = e->_fromCid;
        toCid         = e->_toCid;
        offsetFromKey = e->_offsetFromKey;
        offsetToKey   = e->_offsetToKey;
      }
        
      string const extra = resolver->getCollectionNameCluster(toCid) + 
                           TRI_DOCUMENT_HANDLE_SEPARATOR_CHR + 
                           string(((char*) marker) + offsetToKey) +
                           resolver->getCollectionNameCluster(fromCid) + 
                           TRI_DOCUMENT_HANDLE_SEPARATOR_CHR + 
                           string(((char*) marker) + offsetFromKey);
 
      localCrc += TRI_Crc32HashPointer(extra.c_str(), extra.size());
    }
    else {
      return;
    } 

    if (withData) {
      // with data
      void const* d = static_cast<void const*>(marker);

      TRI_shaped_json_t shaped;
      TRI_EXTRACT_SHAPED_JSON_MARKER(shaped, d);

      TRI_StringifyArrayShapedJson(shaper, &buffer, &shaped, false);  
      localCrc += TRI_Crc32HashPointer(TRI_BeginStringBuffer(&buffer), TRI_LengthStringBuffer(&buffer));
      TRI_ResetStringBuffer(&buffer);
    }

    checksum += localCrc;
  }

  CollectionNameResolver const*  resolver;
  TRI_shaper_t*                  shaper;
  TRI_string_buffer_t            buffer;
  uint32_t                       checksum;
  bool const                     withRevisions;
  bool const                     withData;
    
};

////////////////////////////////////////////////////////////////////////////////
/// @brief calculates a checksum for the data in a collection
/// @startDocuBlock collectionChecksum
/// `collection.checksum(withRevisions, withData)`
///
/// The *checksum* operation calculates a CRC32 checksum of the keys
/// contained in collection *collection*.
///
/// If the optional argument *withRevisions* is set to *true*, then the
/// revision ids of the documents are also included in the checksumming.
///
/// If the optional argument *withData* is set to *true*, then the
/// actual document data is also checksummed. Including the document data in
/// checksumming will make the calculation slower, but is more accurate.
///
/// **Note**: this method is not available in a cluster.
///
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

static void JS_MvccChecksum (const v8::FunctionCallbackInfo<v8::Value>& args) {
  TransactionBase transBase(true);   // To protect against assertions, FIXME later
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope scope(isolate);

  auto const* collection = TRI_UnwrapClass<TRI_vocbase_col_t>(args.Holder(), TRI_GetVocBaseColType());

  if (collection == nullptr) {
    TRI_V8_THROW_EXCEPTION_INTERNAL("cannot extract collection");
  }
  
  if (ServerState::instance()->isCoordinator()) {
    // renaming a collection in a cluster is unsupported
    TRI_V8_THROW_EXCEPTION(TRI_ERROR_CLUSTER_UNSUPPORTED);
  }

  bool withRevisions = false;
  if (args.Length() > 0) {
    withRevisions = TRI_ObjectToBoolean(args[0]);
  }

  bool withData = false;
  if (args.Length() > 1) {
    withData = TRI_ObjectToBoolean(args[1]);
  }

  try {
    triagens::mvcc::TransactionScope transactionScope(collection->_vocbase, triagens::mvcc::TransactionScope::NoCollections());

    auto* transaction = transactionScope.transaction();
    auto* transactionCollection = transaction->collection(collection->_cid);
  
    // extract the index
    CollectionChecksumHelper helper(transaction->resolver(), transactionCollection->shaper(), withRevisions, withData);
  
    {
      std::unique_ptr<triagens::mvcc::MasterpointerIterator> iterator(new triagens::mvcc::MasterpointerIterator(transaction, transactionCollection->masterpointerManager(), false));
    
      auto it = iterator.get();
      while (true) {
        TRI_doc_mptr_t const* found = it->next();

        if (found == nullptr) {
          break;
        }

        helper.update(found);
      }
    }
 
    string const revisionString(std::to_string(triagens::mvcc::CollectionOperations::Revision(&transactionScope, transactionCollection)));

    v8::Handle<v8::Object> result = v8::Object::New(isolate);
    result->Set(TRI_V8_ASCII_STRING("checksum"), v8::Number::New(isolate, helper.checksum));
    result->Set(TRI_V8_ASCII_STRING("revision"), TRI_V8_STD_STRING(revisionString));

    TRI_V8_RETURN(result);
  }
  catch (triagens::arango::Exception const& ex) {
    TRI_V8_THROW_EXCEPTION(ex.code());
  }
  catch (...) {
    TRI_V8_THROW_EXCEPTION(TRI_ERROR_INTERNAL);
  }

  TRI_ASSERT(false);
}


// -----------------------------------------------------------------------------
// --SECTION--                                                     edges queries
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief worker function for querying the edge index
////////////////////////////////////////////////////////////////////////////////
    
static int AddEdges (v8::Isolate* isolate, 
                     TRI_edge_direction_e direction, 
                     triagens::mvcc::Transaction* transaction,
                     triagens::mvcc::TransactionCollection* transactionCollection,
                     triagens::mvcc::EdgeIndex* edgeIndex,
                     v8::Handle<v8::Array>& result,
                     v8::Handle<v8::Value> const vertex) {

  auto resolver = transaction->resolver();

  TRI_voc_cid_t cid;
  std::unique_ptr<char[]> key;

  int res = TRI_ParseVertex(isolate, resolver, cid, key, vertex);

  if (res != TRI_ERROR_NO_ERROR) {
    return res;
  }

  uint32_t i = result->Length();

  TRI_edge_header_t const edge = { cid, key.get() }; 
  std::unique_ptr<std::vector<TRI_doc_mptr_t*>> found(edgeIndex->lookup(transaction, direction, &edge, 0));

  if (found.get() == nullptr) {
    return TRI_ERROR_NO_ERROR;
  }

  auto edges = (*found.get());

  if (i == 0 && ! edges.empty()) {
    result = v8::Array::New(isolate, static_cast<uint32_t>(edges.size()));
  }

  for (auto const& it : edges) {
    v8::Handle<v8::Value> document = TRI_WrapShapedJson(isolate, resolver, transactionCollection, it->getDataPtr());

    if (document.IsEmpty()) {
      return TRI_ERROR_OUT_OF_MEMORY;
    }
          
    result->Set(i++, document);
  }

  return TRI_ERROR_NO_ERROR;
}
  
////////////////////////////////////////////////////////////////////////////////
/// @brief queries the edge index
////////////////////////////////////////////////////////////////////////////////

static void MvccEdgesQuery (TRI_edge_direction_e direction,
                            const v8::FunctionCallbackInfo<v8::Value>& args) {
  TransactionBase transBase(true);   // To protect against assertions, FIXME later
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope scope(isolate);

  auto const* collection = TRI_UnwrapClass<TRI_vocbase_col_t>(args.Holder(), TRI_GetVocBaseColType());

  if (collection == nullptr) {
    TRI_V8_THROW_EXCEPTION_INTERNAL("cannot extract collection");
  }

  TRI_THROW_SHARDING_COLLECTION_NOT_YET_IMPLEMENTED(collection);
  
  if (collection->_type != TRI_COL_TYPE_EDGE) {
    TRI_V8_THROW_EXCEPTION(TRI_ERROR_ARANGO_COLLECTION_TYPE_INVALID);
  }
  
  // first and only argument schould be a list of document idenfifier
  if (args.Length() != 1) {
    switch (direction) {
      case TRI_EDGE_IN:
        TRI_V8_THROW_EXCEPTION_USAGE("inEdges(<vertices>)");

      case TRI_EDGE_OUT:
        TRI_V8_THROW_EXCEPTION_USAGE("outEdges(<vertices>)");

      case TRI_EDGE_ANY:
      default: {
        TRI_V8_THROW_EXCEPTION_USAGE("edges(<vertices>)");
      }
    }
  }
    
  try {
    triagens::mvcc::TransactionScope transactionScope(collection->_vocbase, triagens::mvcc::TransactionScope::NoCollections());

    auto* transaction = transactionScope.transaction();
    auto* transactionCollection = transaction->collection(collection->_cid);
  
    auto edgeIndex = static_cast<triagens::mvcc::EdgeIndex*>(transactionCollection->documentCollection()->lookupIndex(TRI_IDX_TYPE_EDGE_INDEX));

    if (edgeIndex == nullptr) {
      // collection must have an edge index
      TRI_V8_THROW_EXCEPTION(TRI_ERROR_INTERNAL);
    }

    v8::Handle<v8::Array> result = v8::Array::New(isolate);

    if (args[0]->IsArray()) {
      v8::Handle<v8::Array> vertices = v8::Handle<v8::Array>::Cast(args[0]);
      uint32_t const length = vertices->Length();

      for (uint32_t i = 0; i < length; ++i) {
        int res = AddEdges(isolate, direction, transaction, transactionCollection, edgeIndex, result, vertices->Get(i));

        if (res != TRI_ERROR_NO_ERROR) {
          // ignore error
          continue;
        }
      }
    }
    else {
      int res = AddEdges(isolate, direction, transaction, transactionCollection, edgeIndex, result, args[0]);

      if (res != TRI_ERROR_NO_ERROR && res != TRI_ERROR_ARANGO_COLLECTION_NOT_FOUND) {
        // do not ignore error
        TRI_V8_THROW_EXCEPTION(res);
      }
    }

    TRI_V8_RETURN(result);
  }
  catch (triagens::arango::Exception const& ex) {
    TRI_V8_THROW_EXCEPTION(ex.code());
  }
  catch (...) {
    TRI_V8_THROW_EXCEPTION(TRI_ERROR_INTERNAL);
  }

  TRI_ASSERT(false);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief selects all edges for a set of vertices
/// @startDocuBlock edgeCollectionEdges
/// `edge-collection.edges(vertex)`
///
/// The *edges* operator finds all edges starting from (outbound) or ending
/// in (inbound) *vertex*.
///
/// `edge-collection.edges(vertices)`
///
/// The *edges* operator finds all edges starting from (outbound) or ending
/// in (inbound) a document from *vertices*, which must a list of documents
/// or document handles.
///
/// @EXAMPLES
///
/// @EXAMPLE_ARANGOSH_OUTPUT{edgeCollectionEdges}
/// ~ db._create("example");
///   db.relation.edges("vertex/1593622");
/// ~ db._drop("example");
/// @END_EXAMPLE_ARANGOSH_OUTPUT
///
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

static void JS_MvccEdges (const v8::FunctionCallbackInfo<v8::Value>& args) {
  MvccEdgesQuery(TRI_EDGE_ANY, args);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief selects all inbound edges
/// @startDocuBlock edgeCollectionInEdges
/// `edge-collection.inEdges(vertex)`
///
/// The *edges* operator finds all edges ending in (inbound) *vertex*.
///
/// `edge-collection.inEdges(vertices)`
///
/// The *edges* operator finds all edges ending in (inbound) a document from
/// *vertices*, which must a list of documents or document handles.
///
/// @EXAMPLES
///
/// @EXAMPLE_ARANGOSH_OUTPUT{edgeCollectionInEdges}
/// ~ db._create("example");
///   db.relation.inEdges("vertex/1528086");
///   db.relation.inEdges("vertex/1593622");
/// ~ db._drop("example");
/// @END_EXAMPLE_ARANGOSH_OUTPUT
///
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

static void JS_MvccInEdges (const v8::FunctionCallbackInfo<v8::Value>& args) {
  MvccEdgesQuery(TRI_EDGE_IN, args);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief selects all outbound edges
/// @startDocuBlock edgeCollectionOutEdges
/// `edge-collection.outEdges(vertex)`
///
/// The *edges* operator finds all edges starting from (outbound)
/// *vertices*.
///
/// `edge-collection.outEdges(vertices)`
///
/// The *edges* operator finds all edges starting from (outbound) a document
/// from *vertices*, which must a list of documents or document handles.
///
/// @EXAMPLES
///
/// @EXAMPLE_ARANGOSH_OUTPUT{edgeCollectionOutEdges}
/// ~ db._create("example");
///   db.relation.inEdges("vertex/1528086");
///   db.relation.inEdges("vertex/1593622");
/// ~ db._drop("example");
/// @END_EXAMPLE_ARANGOSH_OUTPUT
///
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

static void JS_MvccOutEdges (const v8::FunctionCallbackInfo<v8::Value>& args) {
  MvccEdgesQuery(TRI_EDGE_OUT, args);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  temporal queries
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief helper function for MvccFirst and MvccLast
////////////////////////////////////////////////////////////////////////////////
  
static void MvccTemporalQuery (const v8::FunctionCallbackInfo<v8::Value>& args,
                               bool reverse) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope scope(isolate);
  
  triagens::mvcc::SearchOptions searchOptions;
  searchOptions.skip    = 0;
  searchOptions.limit   = 1;
  searchOptions.reverse = reverse;
  
  bool returnArray = false;

  // if argument is supplied, we return an array - otherwise we simply return the first doc
  if (args.Length() == 1) {
    if (! args[0]->IsUndefined()) {
      searchOptions.limit = TRI_ObjectToInt64(args[0]);
      returnArray = true;
    }
  }

  if (searchOptions.limit < 1) {
    TRI_V8_THROW_EXCEPTION_PARAMETER("invalid value for <count>");
  }
  

  auto const* collection = TRI_UnwrapClass<TRI_vocbase_col_t>(args.Holder(), TRI_GetVocBaseColType());

  if (collection == nullptr) {
    TRI_V8_THROW_EXCEPTION_INTERNAL("cannot extract collection");
  }
  
  // need a fake old transaction in order to not throw - can be removed later       
  TransactionBase oldTrx(true);
  
  triagens::mvcc::OperationOptions options;
  options.searchOptions = &searchOptions;

  try {
    triagens::mvcc::TransactionScope transactionScope(collection->_vocbase, triagens::mvcc::TransactionScope::NoCollections());

    auto* transaction = transactionScope.transaction();
    auto* transactionCollection = transaction->collection(collection->_cid);
    auto resolver = transaction->resolver();

    std::vector<TRI_doc_mptr_t const*> foundDocuments;
    auto searchResult = triagens::mvcc::CollectionOperations::ReadAllDocuments(&transactionScope, transactionCollection, foundDocuments, options);
 
    if (searchResult.code != TRI_ERROR_NO_ERROR) {
      THROW_ARANGO_EXCEPTION(searchResult.code);
    }
  
    if (returnArray) {
      size_t const n = foundDocuments.size();
      auto result = v8::Array::New(isolate, static_cast<int>(n));

      for (size_t i = 0; i < n; ++i) {
        v8::Handle<v8::Value> document = TRI_WrapShapedJson(isolate, resolver, transactionCollection, foundDocuments[i]->getDataPtr());

        if (document.IsEmpty()) {
          TRI_V8_THROW_EXCEPTION_MEMORY();
        }
        result->Set(static_cast<uint32_t>(i), document);
      }

      TRI_V8_RETURN(result);
    }
      
    if (foundDocuments.empty()) {
      TRI_V8_RETURN_NULL();
    }
      
    v8::Handle<v8::Value> result = TRI_WrapShapedJson(isolate, resolver, transactionCollection, foundDocuments[0]->getDataPtr());
    
    if (result.IsEmpty()) {
      TRI_V8_THROW_EXCEPTION_MEMORY();
    }
     
    TRI_V8_RETURN(result);
  }
  catch (triagens::arango::Exception const& ex) {
    TRI_V8_THROW_EXCEPTION_MESSAGE(ex.code(), ex.what());
  }
  catch (...) {
    TRI_V8_THROW_EXCEPTION(TRI_ERROR_INTERNAL);
  }
 
  // unreachable
  TRI_ASSERT(false);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief selects the n first documents in the collection
////////////////////////////////////////////////////////////////////////////////

static void JS_MvccFirst (const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope scope(isolate);

  if (args.Length() > 1) {
    TRI_V8_THROW_EXCEPTION_USAGE("first(<count>)");
  }

  MvccTemporalQuery(args, false);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief selects the n last documents in the collection
////////////////////////////////////////////////////////////////////////////////

static void JS_MvccLast (const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope scope(isolate);

  if (args.Length() > 1) {
    TRI_V8_THROW_EXCEPTION_USAGE("last(<count>)");
  }

  MvccTemporalQuery(args, true);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       geo queries
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief selects documents around a reference coordinate
////////////////////////////////////////////////////////////////////////////////

static void JS_MvccNear (const v8::FunctionCallbackInfo<v8::Value>& args) {
  TransactionBase transBase(true);   // To protect against assertions, FIXME later
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope scope(isolate);
  
  auto const* collection = TRI_UnwrapClass<TRI_vocbase_col_t>(args.Holder(), TRI_GetVocBaseColType());

  if (collection == nullptr) {
    TRI_V8_THROW_EXCEPTION_INTERNAL("cannot extract collection");
  }
  
  TRI_THROW_SHARDING_COLLECTION_NOT_YET_IMPLEMENTED(collection);

  if (args.Length() != 4) {
    TRI_V8_THROW_EXCEPTION_USAGE("near(<index-handle>, <latitude>, <longitude>, <limit>)");
  }
    
  try {
    triagens::mvcc::TransactionScope transactionScope(collection->_vocbase, triagens::mvcc::TransactionScope::NoCollections());

    auto* transaction = transactionScope.transaction();
    auto* transactionCollection = transaction->collection(collection->_cid);
    auto resolver = transaction->resolver();
  
    // extract the index, and hold a read-lock on the list of indexes while we're using the index
    triagens::mvcc::IndexUser indexUser(transactionCollection);
    auto index = TRI_LookupMvccIndexByHandle(isolate, resolver, collection, args[0]);
  
    if (index == nullptr ||
        (index->type() != TRI_IDX_TYPE_GEO1_INDEX &&
         index->type() != TRI_IDX_TYPE_GEO2_INDEX)) {
      TRI_V8_THROW_EXCEPTION(TRI_ERROR_ARANGO_NO_INDEX);
    }
  
    // extract latitude and longitude
    double latitude = TRI_ObjectToDouble(args[1]);
    double longitude = TRI_ObjectToDouble(args[2]);
  
    // extract the limit
    TRI_voc_ssize_t limit = (TRI_voc_ssize_t) TRI_ObjectToDouble(args[3]);
    if (limit <= 0) {
      limit = 100;
    }
  
    std::unique_ptr<std::vector<std::pair<TRI_doc_mptr_t*, double>>> indexResult(static_cast<triagens::mvcc::GeoIndex*>(index)->near(transaction, latitude, longitude, limit));
    
    // setup result
    v8::Handle<v8::Object> result = v8::Object::New(isolate);
    v8::Handle<v8::Array> documents = v8::Array::New(isolate, indexResult->size());
    v8::Handle<v8::Array> distances = v8::Array::New(isolate, indexResult->size());
 
    uint32_t i = 0;
    for (auto const& it : *indexResult) {
      v8::Handle<v8::Value> document = TRI_WrapShapedJson(isolate, resolver, transactionCollection, it.first->getDataPtr());

      if (document.IsEmpty()) {
        TRI_V8_THROW_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
      }

      documents->Set(i, document);
      distances->Set(i, v8::Number::New(isolate, it.second));
      ++i; 
    }

    result->Set(TRI_V8_ASCII_STRING("documents"), documents);
    result->Set(TRI_V8_ASCII_STRING("distances"), distances);
    TRI_V8_RETURN(result);
  }
  catch (triagens::arango::Exception const& ex) {
    TRI_V8_THROW_EXCEPTION(ex.code());
  }
  catch (...) {
    TRI_V8_THROW_EXCEPTION(TRI_ERROR_INTERNAL);
  }

  TRI_ASSERT(false);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief selects the documents within a radius
////////////////////////////////////////////////////////////////////////////////

static void JS_MvccWithin (const v8::FunctionCallbackInfo<v8::Value>& args) {
  TransactionBase transBase(true);   // To protect against assertions, FIXME later
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope scope(isolate);
  
  auto const* collection = TRI_UnwrapClass<TRI_vocbase_col_t>(args.Holder(), TRI_GetVocBaseColType());

  if (collection == nullptr) {
    TRI_V8_THROW_EXCEPTION_INTERNAL("cannot extract collection");
  }
  
  TRI_THROW_SHARDING_COLLECTION_NOT_YET_IMPLEMENTED(collection);

  if (args.Length() != 4) {
    TRI_V8_THROW_EXCEPTION_USAGE("within(<index-handle>, <latitude>, <longitude>, <radius>)");
  }
    
  try {
    triagens::mvcc::TransactionScope transactionScope(collection->_vocbase, triagens::mvcc::TransactionScope::NoCollections());

    auto* transaction = transactionScope.transaction();
    auto* transactionCollection = transaction->collection(collection->_cid);
    auto resolver = transaction->resolver();
  
    // extract the index, and hold a read-lock on the list of indexes while we're using the index
    triagens::mvcc::IndexUser indexUser(transactionCollection);
    auto index = TRI_LookupMvccIndexByHandle(isolate, resolver, collection, args[0]);
  
    if (index == nullptr ||
        (index->type() != TRI_IDX_TYPE_GEO1_INDEX &&
         index->type() != TRI_IDX_TYPE_GEO2_INDEX)) {
      TRI_V8_THROW_EXCEPTION(TRI_ERROR_ARANGO_NO_INDEX);
    }
  
    // extract latitude and longitude
    double latitude = TRI_ObjectToDouble(args[1]);
    double longitude = TRI_ObjectToDouble(args[2]);
    double radius = TRI_ObjectToDouble(args[3]);
  
    std::unique_ptr<std::vector<std::pair<TRI_doc_mptr_t*, double>>> indexResult(static_cast<triagens::mvcc::GeoIndex*>(index)->within(transaction, latitude, longitude, radius));
    
    // setup result
    v8::Handle<v8::Object> result = v8::Object::New(isolate);
    v8::Handle<v8::Array> documents = v8::Array::New(isolate, indexResult->size());
    v8::Handle<v8::Array> distances = v8::Array::New(isolate, indexResult->size());
 
    uint32_t i = 0;
    for (auto const& it : *indexResult) {
      v8::Handle<v8::Value> document = TRI_WrapShapedJson(isolate, resolver, transactionCollection, it.first->getDataPtr());

      if (document.IsEmpty()) {
        TRI_V8_THROW_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
      }

      documents->Set(i, document);
      distances->Set(i, v8::Number::New(isolate, it.second));
      ++i; 
    }

    result->Set(TRI_V8_ASCII_STRING("documents"), documents);
    result->Set(TRI_V8_ASCII_STRING("distances"), distances);
    TRI_V8_RETURN(result);
  }
  catch (triagens::arango::Exception const& ex) {
    TRI_V8_THROW_EXCEPTION(ex.code());
  }
  catch (...) {
    TRI_V8_THROW_EXCEPTION(TRI_ERROR_INTERNAL);
  }

  TRI_ASSERT(false);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                    fulltext query
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief queries the fulltext index
/// @startDocuBlock collectionFulltext
/// `collection.fulltext(attribute, query)`
///
/// The *FULLTEXT* operator performs a fulltext search on the specified
/// *attribute* and the specified *query*.
///
/// Details about the fulltext query syntax can be found below.
///
/// @EXAMPLES
///
/// @EXAMPLE_ARANGOSH_OUTPUT{collectionFulltext}
/// ~ db._drop("emails");
/// ~ db._create("emails");
///   db.emails.ensureFulltextIndex("content").id;
///   db.emails.save({ content: "Hello Alice, how are you doing? Regards, Bob" });
///   db.emails.save({ content: "Hello Charlie, do Alice and Bob know about it?" });
///   db.emails.save({ content: "I think they don't know. Regards, Eve" });
///   db.emails.fulltext("content", "charlie,|eve").toArray();
/// ~ db._drop("emails");
/// @END_EXAMPLE_ARANGOSH_OUTPUT
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

static void JS_MvccFulltext (const v8::FunctionCallbackInfo<v8::Value>& args) {
  TransactionBase transBase(true);   // To protect against assertions, FIXME later
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope scope(isolate);
  
  auto const* collection = TRI_UnwrapClass<TRI_vocbase_col_t>(args.Holder(), TRI_GetVocBaseColType());

  if (collection == nullptr) {
    TRI_V8_THROW_EXCEPTION_INTERNAL("cannot extract collection");
  }
  
  TRI_THROW_SHARDING_COLLECTION_NOT_YET_IMPLEMENTED(collection);

  if (args.Length() != 2) {
    TRI_V8_THROW_EXCEPTION_USAGE("fulltext(<index-handle>, <query>");
  }
    
  try {
    triagens::mvcc::TransactionScope transactionScope(collection->_vocbase, triagens::mvcc::TransactionScope::NoCollections());

    auto* transaction = transactionScope.transaction();
    auto* transactionCollection = transaction->collection(collection->_cid);
    auto resolver = transaction->resolver();
  
    // extract the index, and hold a read-lock on the list of indexes while we're using the index
    triagens::mvcc::IndexUser indexUser(transactionCollection);
    auto index = TRI_LookupMvccIndexByHandle(isolate, resolver, collection, args[0]);
 
    if (index == nullptr ||
        index->type() != TRI_IDX_TYPE_FULLTEXT_INDEX) {
      TRI_V8_THROW_EXCEPTION(TRI_ERROR_ARANGO_NO_INDEX);
    }
  
    std::string const&& queryString = TRI_ObjectToString(args[1]);
    std::unique_ptr<std::vector<TRI_doc_mptr_t*>> indexResult(static_cast<triagens::mvcc::FulltextIndex*>(index)->query(transaction, queryString));
    
    // setup result
    v8::Handle<v8::Array> result = v8::Array::New(isolate, indexResult->size());
 
    uint32_t i = 0;
    for (auto const& it : *indexResult) {
      v8::Handle<v8::Value> document = TRI_WrapShapedJson(isolate, resolver, transactionCollection, it->getDataPtr());

      if (document.IsEmpty()) {
        TRI_V8_THROW_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
      }

      result->Set(i++, document);
    }

    TRI_V8_RETURN(result);
  }
  catch (triagens::arango::Exception const& ex) {
    TRI_V8_THROW_EXCEPTION(ex.code());
  }
  catch (...) {
    TRI_V8_THROW_EXCEPTION(TRI_ERROR_INTERNAL);
  }

  TRI_ASSERT(false);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                            MODULE
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief creates the query functions
////////////////////////////////////////////////////////////////////////////////

void TRI_InitV8Queries (v8::Isolate* isolate,
                        v8::Handle<v8::Context> context) {
  v8::HandleScope scope(isolate);

  TRI_GET_GLOBALS();
  TRI_ASSERT(v8g != nullptr);
  TRI_GET_GLOBAL(VocbaseColTempl, v8::ObjectTemplate);

  // initialize query methods
  TRI_AddMethodVocbase(isolate, VocbaseColTempl, TRI_V8_ASCII_STRING("ALL"), JS_MvccAll, true);
  TRI_AddMethodVocbase(isolate, VocbaseColTempl, TRI_V8_ASCII_STRING("ANY"), JS_MvccAny, true);
  TRI_AddMethodVocbase(isolate, VocbaseColTempl, TRI_V8_ASCII_STRING("BY_CONDITION_SKIPLIST"), JS_MvccByConditionSkiplist, true);
  TRI_AddMethodVocbase(isolate, VocbaseColTempl, TRI_V8_ASCII_STRING("BY_EXAMPLE"), JS_MvccByExample, true);
  TRI_AddMethodVocbase(isolate, VocbaseColTempl, TRI_V8_ASCII_STRING("BY_EXAMPLE_HASH"), JS_MvccByExampleHash, true);
  TRI_AddMethodVocbase(isolate, VocbaseColTempl, TRI_V8_ASCII_STRING("BY_EXAMPLE_SKIPLIST"), JS_MvccByExampleSkiplist, true);
  TRI_AddMethodVocbase(isolate, VocbaseColTempl, TRI_V8_ASCII_STRING("checksum"), JS_MvccChecksum);
  TRI_AddMethodVocbase(isolate, VocbaseColTempl, TRI_V8_ASCII_STRING("EDGES"), JS_MvccEdges, true);
  TRI_AddMethodVocbase(isolate, VocbaseColTempl, TRI_V8_ASCII_STRING("FIRST"), JS_MvccFirst, true);
  TRI_AddMethodVocbase(isolate, VocbaseColTempl, TRI_V8_ASCII_STRING("FULLTEXT"), JS_MvccFulltext, true);
  TRI_AddMethodVocbase(isolate, VocbaseColTempl, TRI_V8_ASCII_STRING("INEDGES"), JS_MvccInEdges, true);
  TRI_AddMethodVocbase(isolate, VocbaseColTempl, TRI_V8_ASCII_STRING("LAST"), JS_MvccLast, true);
  TRI_AddMethodVocbase(isolate, VocbaseColTempl, TRI_V8_ASCII_STRING("NEAR"), JS_MvccNear, true);
  TRI_AddMethodVocbase(isolate, VocbaseColTempl, TRI_V8_ASCII_STRING("OUTEDGES"), JS_MvccOutEdges, true);
  TRI_AddMethodVocbase(isolate, VocbaseColTempl, TRI_V8_ASCII_STRING("WITHIN"), JS_MvccWithin, true);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
