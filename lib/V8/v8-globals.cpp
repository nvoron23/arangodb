////////////////////////////////////////////////////////////////////////////////
/// @brief V8 globals
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
/// @author Copyright 2012-2014, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "v8-globals.h"

// -----------------------------------------------------------------------------
// --SECTION--                                                      public types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief constructor
////////////////////////////////////////////////////////////////////////////////

TRI_v8_global_s::TRI_v8_global_s (v8::Isolate* isolate)
  : JSBarriers(),
    JSCollections(),
    JSCursors(),

    AgencyTempl(),
    ClusterInfoTempl(),
    ServerStateTempl(),
    ClusterCommTempl(),
    ArangoErrorTempl(),
    SleepAndRequeueTempl(),
    SleepAndRequeueFuncTempl(),
    GeneralCursorTempl(),
    ShapedJsonTempl(),
    VocbaseColTempl(),
    VocbaseTempl(),

    BufferTempl(),
    FastBufferConstructor(),
    ExecuteFileCallback(),

    BufferConstant(),
    DeleteConstant(),
    GetConstant(),
    HeadConstant(),
    OptionsConstant(),
    PatchConstant(),
    PostConstant(),
    PutConstant(),

    AddressKey(),
    AllowUseDatabaseKey(),
    BodyFromFileKey(),
    BodyKey(),
    ClientKey(),
    ClientTransactionIDKey(),
    CodeKey(),
    CompatibilityKey(),
    ContentTypeKey(),
    CoordTransactionIDKey(),
    DatabaseKey(),
    DoCompactKey(),
    DomainKey(),
    ErrorKey(),
    ErrorMessageKey(),
    ErrorNumKey(),
    HeadersKey(),
    HttpOnlyKey(),
    IdKey(),
    IsSystemKey(),
    IsVolatileKey(),
    JournalSizeKey(),
    KeepNullKey(),
    KeyOptionsKey(),
    LengthKey(),
    LifeTimeKey(),
    MergeArraysKey(),
    NameKey(),
    OperationIDKey(),
    ParametersKey(),
    PathKey(),
    PrefixKey(),
    PortKey(),
    PortTypeKey(),
    ProtocolKey(),
    RequestBodyKey(),
    RequestTypeKey(),
    ResponseCodeKey(),
    SecureKey(),
    ServerKey(),
    ShardIDKey(),
    SilentKey(),
    SleepKey(),
    StatusKey(),
    SuffixKey(),
    TimeoutKey(),
    TransformationsKey(),
    UrlKey(),
    UserKey(),
    ValueKey(),
    VersionKey(),
    WaitForSyncKey(),

    _FromKey(),
    _DbNameKey(),
    _IdKey(),
    _KeyKey(),
    _OldRevKey(),
    _RevKey(),
    _ToKey(),

    _currentRequest(),
    _currentResponse(),
    _transactionContext(nullptr),
    _queryRegistry(nullptr),
    _query(nullptr),
    _server(nullptr),
    _vocbase(nullptr),
    _allowUseDatabase(true),
    _hasDeadObjects(false),
    _applicationV8(nullptr),
    _loader(nullptr),
    _canceled(false)


{
  v8::HandleScope scope(isolate);

  BufferConstant.Reset(isolate, TRI_V8_SYMBOL("Buffer"));
  DeleteConstant.Reset(isolate, TRI_V8_SYMBOL("DELETE"));
  GetConstant.Reset(isolate, TRI_V8_SYMBOL("GET"));
  HeadConstant.Reset(isolate, TRI_V8_SYMBOL("HEAD"));
  OptionsConstant.Reset(isolate, TRI_V8_SYMBOL("OPTIONS"));
  PatchConstant.Reset(isolate, TRI_V8_SYMBOL("PATCH"));
  PostConstant.Reset(isolate, TRI_V8_SYMBOL("POST"));
  PutConstant.Reset(isolate, TRI_V8_SYMBOL("PUT"));

  AddressKey.Reset(isolate, TRI_V8_SYMBOL("address"));
  AllowUseDatabaseKey.Reset(isolate, TRI_V8_SYMBOL("allowUseDatabase"));
  BodyFromFileKey.Reset(isolate, TRI_V8_SYMBOL("bodyFromFile"));
  BodyKey.Reset(isolate, TRI_V8_SYMBOL("body"));
  ClientKey.Reset(isolate, TRI_V8_SYMBOL("client"));
  ClientTransactionIDKey.Reset(isolate, TRI_V8_SYMBOL("clientTransactionID"));
  CodeKey.Reset(isolate, TRI_V8_SYMBOL("code"));
  CompatibilityKey.Reset(isolate, TRI_V8_SYMBOL("compatibility"));
  ContentTypeKey.Reset(isolate, TRI_V8_SYMBOL("contentType"));
  CookiesKey.Reset(isolate, TRI_V8_SYMBOL("cookies"));
  CoordTransactionIDKey.Reset(isolate, TRI_V8_SYMBOL("coordTransactionID"));
  DatabaseKey.Reset(isolate, TRI_V8_SYMBOL("database"));
  DoCompactKey.Reset(isolate, TRI_V8_SYMBOL("doCompact"));
  DomainKey.Reset(isolate, TRI_V8_SYMBOL("domain"));
  ErrorKey.Reset(isolate, TRI_V8_SYMBOL("error"));
  ErrorMessageKey.Reset(isolate, TRI_V8_SYMBOL("errorMessage"));
  ErrorNumKey.Reset(isolate, TRI_V8_SYMBOL("errorNum"));
  HeadersKey.Reset(isolate, TRI_V8_SYMBOL("headers"));
  HttpOnlyKey.Reset(isolate, TRI_V8_SYMBOL("httpOnly"));
  IdKey.Reset(isolate, TRI_V8_SYMBOL("id"));
  IsSystemKey.Reset(isolate, TRI_V8_SYMBOL("isSystem"));
  IsVolatileKey.Reset(isolate, TRI_V8_SYMBOL("isVolatile"));
  JournalSizeKey.Reset(isolate, TRI_V8_SYMBOL("journalSize"));
  KeepNullKey.Reset(isolate, TRI_V8_SYMBOL("keepNull"));
  KeyOptionsKey.Reset(isolate, TRI_V8_SYMBOL("keyOptions"));
  LengthKey.Reset(isolate, TRI_V8_SYMBOL("length"));
  LifeTimeKey.Reset(isolate, TRI_V8_SYMBOL("lifeTime"));
  MergeArraysKey.Reset(isolate, TRI_V8_SYMBOL("mergeArrays"));
  NameKey.Reset(isolate, TRI_V8_SYMBOL("name"));
  OperationIDKey.Reset(isolate, TRI_V8_SYMBOL("operationID"));
  OverwriteKey.Reset(isolate, TRI_V8_SYMBOL("overwrite"));
  ParametersKey.Reset(isolate, TRI_V8_SYMBOL("parameters"));
  PathKey.Reset(isolate, TRI_V8_SYMBOL("path"));
  PrefixKey.Reset(isolate, TRI_V8_SYMBOL("prefix"));
  PortKey.Reset(isolate, TRI_V8_SYMBOL("port"));
  PortTypeKey.Reset(isolate, TRI_V8_SYMBOL("portType"));
  ProtocolKey.Reset(isolate, TRI_V8_SYMBOL("protocol"));
  RequestBodyKey.Reset(isolate, TRI_V8_SYMBOL("requestBody"));
  RequestTypeKey.Reset(isolate, TRI_V8_SYMBOL("requestType"));
  ResponseCodeKey.Reset(isolate, TRI_V8_SYMBOL("responseCode"));
  SecureKey.Reset(isolate, TRI_V8_SYMBOL("secure"));
  ServerKey.Reset(isolate, TRI_V8_SYMBOL("server"));
  ShardIDKey.Reset(isolate, TRI_V8_SYMBOL("shardID"));
  SilentKey.Reset(isolate, TRI_V8_SYMBOL("silent"));
  SleepKey.Reset(isolate, TRI_V8_SYMBOL("sleep"));
  StatusKey.Reset(isolate, TRI_V8_SYMBOL("status"));
  SuffixKey.Reset(isolate, TRI_V8_SYMBOL("suffix"));
  TimeoutKey.Reset(isolate, TRI_V8_SYMBOL("timeout"));
  TransformationsKey.Reset(isolate, TRI_V8_SYMBOL("transformations"));
  UrlKey.Reset(isolate, TRI_V8_SYMBOL("url"));
  UserKey.Reset(isolate, TRI_V8_SYMBOL("user"));
  ValueKey.Reset(isolate, TRI_V8_SYMBOL("value"));
  VersionKey.Reset(isolate, TRI_V8_SYMBOL("version"));
  WaitForSyncKey.Reset(isolate, TRI_V8_SYMBOL("waitForSync"));

  _FromKey.Reset(isolate, TRI_V8_SYMBOL("_from"));
  _DbNameKey.Reset(isolate, TRI_V8_SYMBOL("_dbName"));
  _IdKey.Reset(isolate, TRI_V8_SYMBOL("_id"));
  _KeyKey.Reset(isolate, TRI_V8_SYMBOL("_key"));
  _OldRevKey.Reset(isolate, TRI_V8_SYMBOL("_oldRev"));
  _RevKey.Reset(isolate, TRI_V8_SYMBOL("_rev"));
  _ToKey.Reset(isolate, TRI_V8_SYMBOL("_to"));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destructor
////////////////////////////////////////////////////////////////////////////////

TRI_v8_global_s::~TRI_v8_global_s () {
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  GLOBAL FUNCTIONS
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a global context
////////////////////////////////////////////////////////////////////////////////

TRI_v8_global_t* TRI_CreateV8Globals (v8::Isolate* isolate) {
  TRI_v8_global_t* v8g = static_cast<TRI_v8_global_t*>(isolate->GetData(V8DataSlot));

  TRI_ASSERT(v8g == nullptr);
  v8g = new TRI_v8_global_t(isolate);
  isolate->SetData(V8DataSlot, v8g);

  return v8g;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns a global context
////////////////////////////////////////////////////////////////////////////////

TRI_v8_global_t* TRI_GetV8Globals (v8::Isolate* isolate) {
  TRI_v8_global_t* v8g = static_cast<TRI_v8_global_t*>(isolate->GetData(V8DataSlot));
  if (v8g == nullptr) {
    v8g = TRI_CreateV8Globals(isolate);
  }

  TRI_ASSERT(v8g != nullptr);
  return v8g;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief adds a method to an object
////////////////////////////////////////////////////////////////////////////////

void TRI_AddMethodVocbase (v8::Isolate* isolate,
                           v8::Handle<v8::ObjectTemplate> tpl,
                           v8::Handle<v8::String> name,
                           void(*func)(v8::FunctionCallbackInfo<v8::Value> const&),
                           bool isHidden) {
  if (isHidden) {
    // hidden method
    tpl->Set(name, v8::FunctionTemplate::New(isolate, func), v8::DontEnum);
  }
  else {
    // normal method
    tpl->Set(name, v8::FunctionTemplate::New(isolate, func));
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief adds a global function to the given context
////////////////////////////////////////////////////////////////////////////////

void TRI_AddGlobalFunctionVocbase (v8::Isolate* isolate,
                                   v8::Handle<v8::Context> context,
                                   v8::Handle<v8::String> name,
                                   void(*func)(v8::FunctionCallbackInfo<v8::Value> const&),
                                   bool isHidden) {
  // all global functions are read-only
  if (isHidden) {
    context->Global()->ForceSet(name,
                                v8::FunctionTemplate::New(isolate, func)->GetFunction(),
                                static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontEnum));
  }
  else {
    context->Global()->ForceSet(name,
                                v8::FunctionTemplate::New(isolate, func)->GetFunction(),
                                v8::ReadOnly);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief adds a global function to the given context
////////////////////////////////////////////////////////////////////////////////

void TRI_AddGlobalFunctionVocbase (v8::Isolate* isolate,
                                   v8::Handle<v8::Context> context,
                                   v8::Handle<v8::String> name,
                                   v8::Handle<v8::Function> func,
                                   bool isHidden) {
  // all global functions are read-only
  if (isHidden) {
    context->Global()->ForceSet(name,
                                func,
                                static_cast<v8::PropertyAttribute>(v8::ReadOnly | v8::DontEnum));
  }
  else {
    context->Global()->ForceSet(name,
                                func,
                                v8::ReadOnly);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief adds a global read-only variable to the given context
////////////////////////////////////////////////////////////////////////////////

void TRI_AddGlobalVariableVocbase (v8::Isolate* isolate,
                                   v8::Handle<v8::Context> context,
                                   v8::Handle<v8::String> name,
                                   v8::Handle<v8::Value> value) {
  // all global variables are read-only
  context->Global()->ForceSet(name, value, v8::ReadOnly);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
