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

#ifndef ARANGODB_V8SERVER_V8__TRIGGERS_H
#define ARANGODB_V8SERVER_V8__TRIGGERS_H 1

#include "Basics/Common.h"
#include "VocBase/server.h"
#include "V8/v8-globals.h"
#include "VocBase/triggers.h"
#include "Utils/CollectionNameResolver.h"
#include "v8-vocbase.h"

////////////////////////////////////////////////////////////////////////////////
/// @brief looks up a trigger
////////////////////////////////////////////////////////////////////////////////

TRI_trigger_t* TRI_LookupTriggerByHandle (triagens::arango::CollectionNameResolver const*,
                                          TRI_vocbase_col_t const*,
                                          v8::Handle<v8::Value>,
                                          v8::Handle<v8::Object>*);

////////////////////////////////////////////////////////////////////////////////
/// @brief execute a pre-trigger
////////////////////////////////////////////////////////////////////////////////

int TRI_ExecutePreTrigger (TRI_trigger_t const*,
                           TRI_document_collection_t*,
                           triagens::wal::Marker const*);

void TRI_InitV8TriggersCollection (v8::Handle<v8::Context> context,
                                   TRI_vocbase_t* vocbase,
                                   TRI_v8_global_t* v8g,
                                   v8::Handle<v8::ObjectTemplate> rt);
#endif
