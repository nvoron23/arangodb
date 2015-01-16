////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC index base class
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
/// @author Copyright 2015, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "Index.h"
#include "Basics/json.h"

using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                                       class Index
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new index
////////////////////////////////////////////////////////////////////////////////

Index::Index (TRI_idx_iid_t id,
              TRI_document_collection_t* collection,
              std::vector<std::string> const& fields,
              bool unique,
              bool ignoreNull,
              bool sparse)
  : _id(id),
    _collection(collection),
    _fields(fields),
    _unique(unique),
    _ignoreNull(ignoreNull),
    _sparse(sparse) {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the index
////////////////////////////////////////////////////////////////////////////////

Index::~Index () {
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief cleans up the index - default implementation (does nothing)
////////////////////////////////////////////////////////////////////////////////
      
int Index::cleanup () {
  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief provides a hint for the expected index size - default implementation 
/// (does nothing)
////////////////////////////////////////////////////////////////////////////////
      
int Index::sizeHint (size_t) {
  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a JSON representation of the index
/// base functionality (called from derived classes)
////////////////////////////////////////////////////////////////////////////////

TRI_json_t* Index::toJson (TRI_memory_zone_t* zone) const {
  TRI_json_t* json = TRI_CreateObjectJson(zone);

  if (json != nullptr) {
    std::string id(std::to_string(_id));
    TRI_Insert3ObjectJson(zone, json, "id", TRI_CreateStringCopyJson(zone, id.c_str(), id.size()));
    TRI_Insert3ObjectJson(zone, json, "type", TRI_CreateStringCopyJson(zone, typeName().c_str(), typeName().size()));
    TRI_Insert3ObjectJson(zone, json, "unique", TRI_CreateBooleanJson(zone, _unique));
  }

  return json;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
