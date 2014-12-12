////////////////////////////////////////////////////////////////////////////////
/// @brief triggers
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

#include "triggers.h"

#include "Basics/conversions.h"
#include "Basics/files.h"
#include "Basics/json.h"
#include "Basics/logging.h"
#include "Basics/tri-strings.h"
#include "Basics/JsonHelper.h"
#include "Utils/Exception.h"
#include "VocBase/document-collection.h"
#include "VocBase/server.h"
#include "Wal/LogfileManager.h"
#include "Wal/Marker.h"

// -----------------------------------------------------------------------------
// --SECTION--                                                          TRIGGERS
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------
  
TRI_trigger_t::TRI_trigger_t (TRI_trigger_id_t id,
                              TRI_trigger_type_e type,
                              TRI_trigger_position_e position,
                              std::string const& action) 
  : _internalId(TRI_NewTickServer()),
    _id(id),
    _type(type),
    _position(position),
    _action(action) {
}

TRI_trigger_t::~TRI_trigger_t () {
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a trigger description as JSON
////////////////////////////////////////////////////////////////////////////////

TRI_json_t* TRI_trigger_t::toJson (TRI_memory_zone_t* zone) const {
  TRI_json_t* json = TRI_CreateArrayJson(zone);

  if (json != nullptr) {
    char* number = TRI_StringUInt64(_id);
    TRI_Insert3ArrayJson(zone, json, "id", TRI_CreateStringCopyJson(zone, number));
    TRI_Insert3ArrayJson(zone, json, "type", TRI_CreateStringCopyJson(zone, TRI_TypeTrigger(_type)));
    TRI_Insert3ArrayJson(zone, json, "position", TRI_CreateStringCopyJson(zone, TRI_PositionTrigger(_position)));
    TRI_Insert3ArrayJson(zone, json, "action", TRI_CreateString2CopyJson(zone, _action.c_str(), _action.size()));

    TRI_FreeString(TRI_CORE_MEM_ZONE, number);
  }

  return json;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the trigger type by name
////////////////////////////////////////////////////////////////////////////////

TRI_trigger_type_e TRI_TypeTrigger (char const* type) {
  if (TRI_EqualString(type, "insert")) {
    return TRI_TRIGGER_TYPE_INSERT;
  }
  else if (TRI_EqualString(type, "update")) {
    return TRI_TRIGGER_TYPE_UPDATE;
  }
  else if (TRI_EqualString(type, "replace")) {
    return TRI_TRIGGER_TYPE_REPLACE;
  }
  else if (TRI_EqualString(type, "remove")) {
    return TRI_TRIGGER_TYPE_REMOVE;
  }

  return TRI_TRIGGER_TYPE_UNKNOWN;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the name of a trigger type
////////////////////////////////////////////////////////////////////////////////

char const* TRI_TypeTrigger (TRI_trigger_type_e type) {
  switch (type) {
    case TRI_TRIGGER_TYPE_INSERT:
      return "insert";
    case TRI_TRIGGER_TYPE_UPDATE:
      return "update";
    case TRI_TRIGGER_TYPE_REPLACE:
      return "replace";
    case TRI_TRIGGER_TYPE_REMOVE:
      return "remove";
    case TRI_IDX_TYPE_UNKNOWN:
    default: {
    }
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the trigger position by name
////////////////////////////////////////////////////////////////////////////////

TRI_trigger_position_e TRI_PositionTrigger (char const* position) {
  if (TRI_EqualString(position, "pre")) {
    return TRI_TRIGGER_POSITION_PRE;
  }
  else if (TRI_EqualString(position, "post")) {
    return TRI_TRIGGER_POSITION_POST;
  }
    
  return TRI_TRIGGER_POSITION_UNKNOWN;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the name of a trigger position
////////////////////////////////////////////////////////////////////////////////

char const* TRI_PositionTrigger (TRI_trigger_position_e position) {
  switch (position) {
    case TRI_TRIGGER_POSITION_PRE:
      return "pre";
    case TRI_TRIGGER_POSITION_POST:
      return "post";
    case TRI_TRIGGER_POSITION_UNKNOWN:
    default: {
    }
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
/// @brief validate a trigger id
////////////////////////////////////////////////////////////////////////////////

bool TRI_ValidateIdTrigger (char const* key) {
  char const* p = key;

  while (1) {
    const char c = *p;

    if (c == '\0') {
      return (p - key) > 0;
    }
    if (c >= '0' && c <= '9') {
      ++p;
      continue;
    }

    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief validate a trigger id (collection name + / + trigger id)
////////////////////////////////////////////////////////////////////////////////

bool TRI_ValidateTriggerIdTrigger (char const* key,
                                   size_t* split) {
  char const* p = key;
  char c = *p;

  // extract collection name

  if (! (c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
    return false;
  }

  ++p;

  while (1) {
    c = *p;

    if ((c == '_') || (c == '-') || (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
      ++p;
      continue;
    }

    if (c == '/') {
      break;
    }

    return false;
  }

  if (p - key > TRI_COL_NAME_LENGTH) {
    return false;
  }

  // store split position
  *split = p - key;
  ++p;

  // validate trigger id
  return TRI_ValidateIdTrigger(p);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief remove trigger file
////////////////////////////////////////////////////////////////////////////////

bool TRI_RemoveTriggerFile (TRI_document_collection_t* collection, 
                            TRI_trigger_t* trigger) {
  // construct filename
  char* number = TRI_StringUInt64(trigger->_id);

  if (number == nullptr) {
    TRI_set_errno(TRI_ERROR_OUT_OF_MEMORY);
    LOG_ERROR("out of memory when creating trigger number");
    return false;
  }

  char* name = TRI_Concatenate3String("trigger-", number, ".json");

  if (name == nullptr) {
    TRI_set_errno(TRI_ERROR_OUT_OF_MEMORY);

    TRI_FreeString(TRI_CORE_MEM_ZONE, number);
    LOG_ERROR("out of memory when creating trigger name");
    return false;
  }

  char* filename = TRI_Concatenate2File(collection->_directory, name);

  if (filename == nullptr) {
    TRI_set_errno(TRI_ERROR_OUT_OF_MEMORY);

    TRI_FreeString(TRI_CORE_MEM_ZONE, number);
    TRI_FreeString(TRI_CORE_MEM_ZONE, name);
    LOG_ERROR("out of memory when creating trigger filename");
    return false;
  }

  TRI_FreeString(TRI_CORE_MEM_ZONE, name);
  TRI_FreeString(TRI_CORE_MEM_ZONE, number);

  int res = TRI_UnlinkFile(filename);
  TRI_FreeString(TRI_CORE_MEM_ZONE, filename);

  if (res != TRI_ERROR_NO_ERROR) {
    LOG_ERROR("cannot remove trigger definition: %s", TRI_last_error());
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief saves a trigger
////////////////////////////////////////////////////////////////////////////////

int TRI_SaveTrigger (TRI_document_collection_t* document,
                     TRI_trigger_t* trigger,
                     bool writeMarker) {
  // convert into JSON
  TRI_json_t* json = trigger->toJson(TRI_UNKNOWN_MEM_ZONE);

  if (json == nullptr) {
    LOG_TRACE("cannot save trigger definition: trigger cannot be jsonified");
    return TRI_set_errno(TRI_ERROR_INTERNAL);
  }

  // construct filename
  char* number   = TRI_StringUInt64(trigger->_id);
  char* name     = TRI_Concatenate3String("trigger-", number, ".json");
  char* filename = TRI_Concatenate2File(document->_directory, name);

  TRI_FreeString(TRI_CORE_MEM_ZONE, name);
  TRI_FreeString(TRI_CORE_MEM_ZONE, number);

  TRI_vocbase_t* vocbase = document->_vocbase;

  // and save
  bool ok = TRI_SaveJson(filename, json, document->_vocbase->_settings.forceSyncProperties);

  TRI_FreeString(TRI_CORE_MEM_ZONE, filename);

  if (! ok) {
    LOG_ERROR("cannot save trigger definition: %s", TRI_last_error());
    TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, json);

    return TRI_errno();
  }

  if (! writeMarker) {
    return TRI_ERROR_NO_ERROR;
  }

  int res = TRI_ERROR_NO_ERROR;

  try {
    triagens::wal::CreateTriggerMarker marker(vocbase->_id, document->_info._cid, trigger->_id, triagens::basics::JsonHelper::toString(json));
    triagens::wal::SlotInfoCopy slotInfo = triagens::wal::LogfileManager::instance()->allocateAndWrite(marker, false);

    if (slotInfo.errorCode != TRI_ERROR_NO_ERROR) {
      THROW_ARANGO_EXCEPTION(slotInfo.errorCode);
    }

    TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, json);
    return TRI_ERROR_NO_ERROR;
  }
  catch (triagens::arango::Exception const& ex) {
    res = ex.code();
  }
  catch (...) {
    res = TRI_ERROR_INTERNAL;
  }

  TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, json);

  // TODO: what to do here?
  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief looks up a trigger identifier
////////////////////////////////////////////////////////////////////////////////

TRI_trigger_t* TRI_LookupTrigger (TRI_document_collection_t* document,
                                  TRI_trigger_id_t tid) {
  size_t const n = document->_triggers._length;

  for (size_t i = 0;  i < n;  ++i) {
    TRI_trigger_t* trigger = static_cast<TRI_trigger_t*>(document->_triggers._buffer[i]);

    if (trigger->_id == tid) {
      return trigger;
    }
  }

  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief looks up a trigger by type and position
////////////////////////////////////////////////////////////////////////////////

TRI_trigger_t* TRI_LookupByTypeTrigger (TRI_document_collection_t* document,
                                        TRI_trigger_type_e type,
                                        TRI_trigger_position_e position) {
  size_t const n = document->_triggers._length;

  for (size_t i = 0;  i < n;  ++i) {
    TRI_trigger_t* trigger = static_cast<TRI_trigger_t*>(document->_triggers._buffer[i]);

    if (trigger->_type == type && 
        trigger->_position == position) {
      return trigger;
    }
  }

  return nullptr;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
