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

#ifndef ARANGODB_VOC_BASE_TRIGGERS_H
#define ARANGODB_VOC_BASE_TRIGGERS_H 1

#include "Basics/Common.h"

#include "VocBase/vocbase.h"

#include "Basics/json.h"
#include "VocBase/voc-types.h"

// -----------------------------------------------------------------------------
// --SECTION--                                              forward declarations
// -----------------------------------------------------------------------------

struct TRI_document_collection_t;
struct TRI_trigger_t;

// -----------------------------------------------------------------------------
// --SECTION--                                                      public types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief trigger type
////////////////////////////////////////////////////////////////////////////////

typedef enum {
  TRI_TRIGGER_TYPE_UNKNOWN = 0,
  TRI_TRIGGER_TYPE_INSERT,
  TRI_TRIGGER_TYPE_UPDATE,
  TRI_TRIGGER_TYPE_REPLACE,
  TRI_TRIGGER_TYPE_REMOVE
}
TRI_trigger_type_e;

////////////////////////////////////////////////////////////////////////////////
/// @brief trigger position
////////////////////////////////////////////////////////////////////////////////

typedef enum {
  TRI_TRIGGER_POSITION_UNKNOWN = 0,
  TRI_TRIGGER_POSITION_PRE,
  TRI_TRIGGER_POSITION_POST
}
TRI_trigger_position_e;

// -----------------------------------------------------------------------------
// --SECTION--                                                          TRIGGERS
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief trigger class
////////////////////////////////////////////////////////////////////////////////

struct TRI_trigger_t {
  TRI_trigger_id_t const             _internalId;
  TRI_trigger_id_t                   _id;
  TRI_trigger_type_e const           _type;
  TRI_trigger_position_e const       _position;
  std::string const                  _action; 

  TRI_trigger_t (TRI_trigger_id_t,
                 TRI_trigger_type_e,
                 TRI_trigger_position_e,
                 std::string const&);

  ~TRI_trigger_t ();

  TRI_json_t* toJson (TRI_memory_zone_t*) const;
};

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief return the type of a trigger by name
////////////////////////////////////////////////////////////////////////////////

TRI_trigger_type_e TRI_TypeTrigger (char const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief return the name of a trigger type
////////////////////////////////////////////////////////////////////////////////

char const* TRI_TypeTrigger (TRI_trigger_type_e);

////////////////////////////////////////////////////////////////////////////////
/// @brief return the position of a trigger by name
////////////////////////////////////////////////////////////////////////////////

TRI_trigger_position_e TRI_PositionTrigger (char const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief return the name of a trigger position
////////////////////////////////////////////////////////////////////////////////

char const* TRI_PositionTrigger (TRI_trigger_position_e);

////////////////////////////////////////////////////////////////////////////////
/// @brief validate trigger id string
////////////////////////////////////////////////////////////////////////////////

bool TRI_ValidateIdTrigger (char const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief validate an trigger id (collection name + / + trigger id)
////////////////////////////////////////////////////////////////////////////////

bool TRI_ValidateTriggerIdTrigger (char const*,
                                   size_t*);

////////////////////////////////////////////////////////////////////////////////
/// @brief removes a trigger file
////////////////////////////////////////////////////////////////////////////////

bool TRI_RemoveTriggerFile (struct TRI_document_collection_t*,
                            TRI_trigger_t*);

////////////////////////////////////////////////////////////////////////////////
/// @brief saves a trigger
////////////////////////////////////////////////////////////////////////////////

int TRI_SaveTrigger (struct TRI_document_collection_t*,
                     TRI_trigger_t*,
                     bool);

////////////////////////////////////////////////////////////////////////////////
/// @brief looks up a trigger identifier
////////////////////////////////////////////////////////////////////////////////

TRI_trigger_t* TRI_LookupTrigger (struct TRI_document_collection_t*,
                                  TRI_trigger_id_t);

////////////////////////////////////////////////////////////////////////////////
/// @brief looks up a trigger by type and position
////////////////////////////////////////////////////////////////////////////////

TRI_trigger_t* TRI_LookupByTypeTrigger (struct TRI_document_collection_t*,
                                        TRI_trigger_type_e,
                                        TRI_trigger_position_e);

#endif

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
