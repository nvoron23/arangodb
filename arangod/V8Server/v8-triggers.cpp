////////////////////////////////////////////////////////////////////////////////
/// @brief V8-vocbase bridge
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
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "v8-triggers.h"
#include "v8-collection.h"
#include "v8-vocbase.h"
#include "v8-vocbaseprivate.h"
#include "v8-wrapshapedjson.h"

#include "Basics/conversions.h"
#include "Utils/transactions.h"
#include "Utils/V8TransactionContext.h"
#include "V8/v8-conv.h"
#include "V8/v8-utils.h"

using namespace std;
using namespace triagens::basics;
using namespace triagens::arango;
using namespace triagens::rest;

////////////////////////////////////////////////////////////////////////////////
/// @brief execute a pre-trigger
////////////////////////////////////////////////////////////////////////////////

static int ExecutePreTrigger (TRI_trigger_t const* trigger,
                              v8::Handle<v8::Value> doc) { 
  v8::HandleScope scope;
  v8::TryCatch tryCatch;

  v8::Handle<v8::Object> current = v8::Context::GetCurrent()->Global();
  TRI_v8_global_t* v8g = static_cast<TRI_v8_global_t*>(v8::Isolate::GetCurrent()->GetData());
  v8::Handle<v8::Function> action;

  uint64_t const internalId = static_cast<uint64_t>(trigger->_internalId);

  auto it = v8g->JSTriggers.find(internalId);

  if (it == v8g->JSTriggers.end()) {
    // not yet created, now build the trigger
    v8::Local<v8::Function> ctor = v8::Local<v8::Function>::Cast(current->Get(v8::String::New("Function")));

    // Invoke Function constructor to create function with the given body and no arguments
    std::string body = "return (" + trigger->_action + ")(doc);";
    v8::Handle<v8::Value> args[2] = { v8::String::New("doc"), v8::String::New(body.c_str(), (int) body.size()) };
    v8::Local<v8::Object> function = ctor->NewInstance(2, args);

    action = v8::Local<v8::Function>::Cast(function);
    if (action.IsEmpty()) {
      return TRI_ERROR_ARANGO_TRIGGER_FUNCTION_INVALID_CODE; 
    }

    v8::Isolate* isolate = v8::Isolate::GetCurrent();
    v8g->JSTriggers.emplace(std::make_pair(internalId, v8::Persistent<v8::Value>::New(isolate, action)));
  }
  else {
    // trigger found in lookup list
    action = v8::Handle<v8::Function>::Cast((*it).second); 
    if (action.IsEmpty()) {
      return TRI_ERROR_INTERNAL; 
    }
  }

  // now call the trigger
  v8::Handle<v8::Value> args = doc;
  action->Call(current, 1, &args);

  if (tryCatch.HasCaught()) {
    return TRI_ERROR_ARANGO_TRIGGER_RUNTIME_ERROR;
  }

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief checks if argument is a trigger identifier
////////////////////////////////////////////////////////////////////////////////

static bool IsTriggerHandle (v8::Handle<v8::Value> const arg,
                             string& collectionName,
                             TRI_trigger_id_t& tid) {

  TRI_ASSERT(collectionName.empty());
  TRI_ASSERT(tid == 0);

  if (arg->IsNumber()) {
    // numeric trigger id
    tid = (TRI_trigger_id_t) arg->ToNumber()->Value();
    return true;
  }

  if (! arg->IsString()) {
    return false;
  }

  v8::String::Utf8Value str(arg);

  if (*str == 0) {
    return false;
  }

  size_t split;
  if (TRI_ValidateTriggerIdTrigger(*str, &split)) {
    collectionName = string(*str, split);
    tid = TRI_UInt64String2(*str + split + 1, str.length() - split - 1);
    return true;
  }

  if (TRI_ValidateIdTrigger(*str)) {
    tid = TRI_UInt64String2(*str, str.length());
    return true;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the trigger representation
////////////////////////////////////////////////////////////////////////////////

static v8::Handle<v8::Value> TriggerRep (std::string const& collectionName,
                                         TRI_trigger_t const* trigger) {
  v8::HandleScope scope;
  
  v8::Handle<v8::Object> rep;

  TRI_json_t* json = trigger->toJson(TRI_UNKNOWN_MEM_ZONE);

  if (json != nullptr) {
    rep = TRI_ObjectJson(json)->ToObject();
  
    string triggerId = TRI_ObjectToString(rep->Get(TRI_V8_SYMBOL("id")));
    string const id = collectionName + TRI_INDEX_HANDLE_SEPARATOR_STR + triggerId;
    rep->Set(TRI_V8_SYMBOL("id"), v8::String::New(id.c_str(), (int) id.size()));

    TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, json);
  }

  return scope.Close(rep);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a trigger, locally
////////////////////////////////////////////////////////////////////////////////

static v8::Handle<v8::Value> CreateTriggerLocal (TRI_vocbase_col_t const* collection,
                                                 TRI_json_t const* json) {
  v8::HandleScope scope;

  TRI_ASSERT(collection != nullptr);
  TRI_ASSERT(json != nullptr);

  // extract type
  TRI_json_t* value = TRI_LookupArrayJson(json, "type");
  if (! TRI_IsStringJson(value)) {
    TRI_V8_EXCEPTION_PARAMETER(scope, "<trigger-data>.type must be a string");
  }

  TRI_trigger_type_e type = TRI_TypeTrigger(value->_value._string.data);

  // extract position
  value = TRI_LookupArrayJson(json, "position");
  if (! TRI_IsStringJson(value)) {
    TRI_V8_EXCEPTION_PARAMETER(scope, "<trigger-data>.position must be a string");
  }

  TRI_trigger_position_e position = TRI_PositionTrigger(value->_value._string.data);

  // extract action
  value = TRI_LookupArrayJson(json, "action");
  if (! TRI_IsStringJson(value)) {
    TRI_V8_EXCEPTION_PARAMETER(scope, "<trigger-data>.action must be a string");
  }

  std::string action(value->_value._string.data, value->_value._string.length - 1);

  // extract id
  TRI_trigger_id_t tid = 0;
  value = TRI_LookupArrayJson(json, "id");
  if (TRI_IsStringJson(value)) {
    tid = TRI_UInt64String2(value->_value._string.data, value->_value._string.length - 1);
  }
  else {
    tid = TRI_NewTickServer();
  }

  SingleCollectionReadOnlyTransaction trx(new V8TransactionContext(true), collection->_vocbase, collection->_cid);

  int res = trx.begin();

  if (res != TRI_ERROR_NO_ERROR) {
    TRI_V8_EXCEPTION(scope, res);
  }


  TRI_document_collection_t* document = trx.documentCollection();
  std::string const collectionName(collection->_name);

  // disallow trigger creation in read-only mode
  if (! TRI_IsSystemNameCollection(collectionName.c_str()) 
      && TRI_GetOperationModeServer() == TRI_VOCBASE_MODE_NO_CREATE) {
    TRI_V8_EXCEPTION(scope, TRI_ERROR_ARANGO_READ_ONLY);
  }

  TRI_trigger_t* trigger = TRI_CreateTriggerDocumentCollection(document,
                                                               tid,
                                                               type,
                                                               position,
                                                               action,
                                                               res);

  if (trigger == nullptr) {
    // something went wrong during creation
    TRI_V8_EXCEPTION(scope, res);
  }

  v8::Handle<v8::Value> ret = TriggerRep(collectionName, trigger);

  return scope.Close(ret);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a trigger
////////////////////////////////////////////////////////////////////////////////

static v8::Handle<v8::Value> JS_CreateTriggerVocbaseCol (v8::Arguments const& argv) {
  v8::HandleScope scope;

  PREVENT_EMBEDDED_TRANSACTION(scope);

  TRI_vocbase_col_t* collection = TRI_UnwrapClass<TRI_vocbase_col_t>(argv.Holder(), WRP_VOCBASE_COL_TYPE);

  if (collection == nullptr) {
    TRI_V8_EXCEPTION_INTERNAL(scope, "cannot extract collection");
  }

  if (argv.Length() != 1 || ! argv[0]->IsObject()) {
    TRI_V8_EXCEPTION_USAGE(scope, "createTrigger(<object>)");
  }

  v8::Handle<v8::Value> ret;

  // ensure an trigger, coordinator case
  if (ServerState::instance()->isCoordinator()) { 
    TRI_V8_EXCEPTION(scope, TRI_ERROR_NOT_IMPLEMENTED);
  }
  else {
    TRI_json_t* json = TRI_ObjectToJson(argv[0]);

    if (json == nullptr) {
      TRI_V8_EXCEPTION_MEMORY(scope);
    }

    ret = CreateTriggerLocal(collection, json);
    TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, json);
  }

  return scope.Close(ret);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief drops a trigger
////////////////////////////////////////////////////////////////////////////////

static v8::Handle<v8::Value> JS_DropTriggerVocbaseCol (v8::Arguments const& argv) {
  v8::HandleScope scope;

  PREVENT_EMBEDDED_TRANSACTION(scope);

  TRI_vocbase_col_t* collection = TRI_UnwrapClass<TRI_vocbase_col_t>(argv.Holder(), WRP_VOCBASE_COL_TYPE);

  if (collection == nullptr) {
    TRI_V8_EXCEPTION_INTERNAL(scope, "cannot extract collection");
  }

  if (argv.Length() != 1) {
    TRI_V8_EXCEPTION_USAGE(scope, "dropTrigger(<trigger-handle>)");
  }

  if (ServerState::instance()->isCoordinator()) {
    TRI_V8_EXCEPTION(scope, TRI_ERROR_NOT_IMPLEMENTED);
  }

  SingleCollectionReadOnlyTransaction trx(new V8TransactionContext(true), collection->_vocbase, collection->_cid);

  int res = trx.begin();

  if (res != TRI_ERROR_NO_ERROR) {
    TRI_V8_EXCEPTION(scope, res);
  }

  TRI_document_collection_t* document = trx.documentCollection();

  v8::Handle<v8::Object> err;
  TRI_trigger_t* trigger = TRI_LookupTriggerByHandle(trx.resolver(), collection, argv[0], &err);

  if (trigger == nullptr) {
    if (err.IsEmpty()) {
      return scope.Close(v8::False());
    }
    else {
      return scope.Close(v8::ThrowException(err));
    }
  }

  // .............................................................................
  // inside a write transaction, write-lock is acquired by TRI_DropTrigger...
  // .............................................................................

  bool ok = TRI_DropTriggerDocumentCollection(document, trigger->_id, true);

  // .............................................................................
  // outside a write transaction
  // .............................................................................

  return scope.Close(v8::Boolean::New(ok));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns information about the triggers
////////////////////////////////////////////////////////////////////////////////

static v8::Handle<v8::Value> JS_GetTriggersVocbaseCol (v8::Arguments const& argv) {
  v8::HandleScope scope;

  TRI_vocbase_col_t* collection = TRI_UnwrapClass<TRI_vocbase_col_t>(argv.Holder(), WRP_VOCBASE_COL_TYPE);

  if (collection == nullptr) {
    TRI_V8_EXCEPTION_INTERNAL(scope, "cannot extract collection");
  }

  if (ServerState::instance()->isCoordinator()) {
    TRI_V8_EXCEPTION(scope, TRI_ERROR_NOT_IMPLEMENTED);
  }

  SingleCollectionReadOnlyTransaction trx(new V8TransactionContext(true), collection->_vocbase, collection->_cid);

  int res = trx.begin();

  if (res != TRI_ERROR_NO_ERROR) {
    TRI_V8_EXCEPTION(scope, res);
  }

  // READ-LOCK start
  trx.lockRead();

  TRI_document_collection_t* document = trx.documentCollection();
  std::string const collectionName(collection->_name);

  // get list of triggers
  auto triggers = TRI_TriggersDocumentCollection(document);

  trx.finish(res);
  // READ-LOCK end

  size_t const n = triggers.size();
  v8::Handle<v8::Array> result = v8::Array::New(n);

  for (size_t i = 0; i < n;  ++i) {
    result->Set(static_cast<uint32_t>(i), TriggerRep(collectionName, triggers[i]));
  }

  return scope.Close(result);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief looks up a trigger identifier
////////////////////////////////////////////////////////////////////////////////

TRI_trigger_t* TRI_LookupTriggerByHandle (CollectionNameResolver const* resolver,
                                          TRI_vocbase_col_t const* collection,
                                          v8::Handle<v8::Value> const val,
                                          v8::Handle<v8::Object>* err) {
  // reset the collection identifier
  string collectionName;
  TRI_trigger_id_t tid = 0;

  // assume we are already loaded
  TRI_ASSERT(collection != nullptr);
  TRI_ASSERT(collection->_collection != nullptr);

  // extract the trigger identifier from a string
  if (val->IsString() || val->IsStringObject() || val->IsNumber()) {
    if (! IsTriggerHandle(val, collectionName, tid)) {
      *err = TRI_CreateErrorObject(__FILE__, __LINE__, TRI_ERROR_ARANGO_INDEX_HANDLE_BAD);
      return nullptr;
    }
  }

  // extract the trigger identifier from an object
  else if (val->IsObject()) {
    TRI_v8_global_t* v8g = static_cast<TRI_v8_global_t*>(v8::Isolate::GetCurrent()->GetData());

    v8::Handle<v8::Object> obj = val->ToObject();
    v8::Handle<v8::Value> tidVal = obj->Get(v8g->IdKey);

    if (! IsTriggerHandle(tidVal, collectionName, tid)) {
      *err = TRI_CreateErrorObject(__FILE__, __LINE__, TRI_ERROR_ARANGO_INDEX_HANDLE_BAD);
      return nullptr;
    }
  }

  if (! collectionName.empty()) {
    if (! EqualCollection(resolver, collectionName, collection)) {
      // I wish this error provided me with more information!
      // e.g. 'cannot access trigger outside the collection it was defined in'
      *err = TRI_CreateErrorObject(__FILE__, __LINE__, TRI_ERROR_ARANGO_CROSS_COLLECTION_REQUEST);
      return nullptr;
    }
  }

  TRI_trigger_t* trigger = TRI_LookupTrigger(collection->_collection, tid);

  if (trigger == nullptr) {
    *err = TRI_CreateErrorObject(__FILE__, __LINE__, TRI_ERROR_ARANGO_INDEX_NOT_FOUND);
  }

  return trigger;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief execute a pre-trigger
////////////////////////////////////////////////////////////////////////////////

int TRI_ExecutePreTrigger (TRI_trigger_t const* trigger,
                           TRI_document_collection_t* document,
                           triagens::wal::Marker const* marker) {
  v8::HandleScope scope;

  v8::Handle<v8::Value> doc = TRI_CreateDocumentFromMarkerV8(document, marker);
  if (doc.IsEmpty()) {
    return TRI_ERROR_INTERNAL;
  }

  return ExecutePreTrigger(trigger, doc);
}

void TRI_InitV8TriggersCollection (v8::Handle<v8::Context> context,
                                   TRI_vocbase_t* vocbase,
                                   TRI_v8_global_t* v8g,
                                   v8::Handle<v8::ObjectTemplate> rt) {

  TRI_AddMethodVocbase(rt, "dropTrigger", JS_DropTriggerVocbaseCol);
  TRI_AddMethodVocbase(rt, "createTrigger", JS_CreateTriggerVocbaseCol);
  TRI_AddMethodVocbase(rt, "getTriggers", JS_GetTriggersVocbaseCol);
}
