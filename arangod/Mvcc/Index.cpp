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
#include "Basics/json-utilities.h"
#include "VocBase/server.h"
#include "VocBase/vocbase.h"

using namespace triagens::basics;
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
              std::vector<std::string> const& fields)
  : _id(id),
    _fields(fields) {

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
      
void Index::cleanup () {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief provides a hint for the expected index size - default implementation 
/// (does nothing)
////////////////////////////////////////////////////////////////////////////////
      
void Index::sizeHint (size_t) {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a JSON representation of the figures of the index
////////////////////////////////////////////////////////////////////////////////

triagens::basics::Json Index::figures () const {
  triagens::basics::Json json(TRI_UNKNOWN_MEM_ZONE, triagens::basics::Json::Object);
  return json;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a JSON representation of the index
/// base functionality (called from derived classes)
////////////////////////////////////////////////////////////////////////////////

triagens::basics::Json Index::toJson (TRI_memory_zone_t* zone) const {
  triagens::basics::Json json(zone, triagens::basics::Json::Object, 4);
  json("id", triagens::basics::Json(zone, static_cast<double>(_id)))
      ("type", triagens::basics::Json(zone, typeName()));
  return json;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the index type based on a type name
////////////////////////////////////////////////////////////////////////////////

Index::IndexType Index::type (char const* type) {
  if (::strcmp(type, "primary") == 0) {
    return TRI_IDX_TYPE_PRIMARY_INDEX;
  }
  if (::strcmp(type, "edge") == 0) {
    return TRI_IDX_TYPE_EDGE_INDEX;
  }
  if (::strcmp(type, "hash") == 0) {
    return TRI_IDX_TYPE_HASH_INDEX;
  }
  if (::strcmp(type, "skiplist") == 0) {
    return TRI_IDX_TYPE_SKIPLIST_INDEX;
  }
  if (::strcmp(type, "fulltext") == 0) {
    return TRI_IDX_TYPE_FULLTEXT_INDEX;
  }
  if (::strcmp(type, "cap") == 0) {
    return TRI_IDX_TYPE_CAP_CONSTRAINT;
  }
  if (::strcmp(type, "geo1") == 0) {
    return TRI_IDX_TYPE_GEO1_INDEX;
  }
  if (::strcmp(type, "geo2") == 0) {
    return TRI_IDX_TYPE_GEO2_INDEX;
  }

  return TRI_IDX_TYPE_UNKNOWN;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the name of an index type
////////////////////////////////////////////////////////////////////////////////

char const* Index::type (Index::IndexType type) {
  switch (type) {
    case TRI_IDX_TYPE_PRIMARY_INDEX:
      return "primary";
    case TRI_IDX_TYPE_GEO1_INDEX:
      return "geo1";
    case TRI_IDX_TYPE_GEO2_INDEX:
      return "geo2";
    case TRI_IDX_TYPE_HASH_INDEX:
      return "hash";
    case TRI_IDX_TYPE_EDGE_INDEX:
      return "edge";
    case TRI_IDX_TYPE_FULLTEXT_INDEX:
      return "fulltext";
    case TRI_IDX_TYPE_SKIPLIST_INDEX:
      return "skiplist";
    case TRI_IDX_TYPE_CAP_CONSTRAINT:
      return "cap";
    case TRI_IDX_TYPE_PRIORITY_QUEUE_INDEX:
    case TRI_IDX_TYPE_BITARRAY_INDEX:
    case TRI_IDX_TYPE_UNKNOWN: {
    }
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////
/// @brief generate a new index id
////////////////////////////////////////////////////////////////////////////////

TRI_idx_iid_t Index::generateId () {
  return TRI_NewTickServer();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief validate an index id
////////////////////////////////////////////////////////////////////////////////

bool Index::validateId (char const* key) {
  char const* p = key;

  while (true) {
    char const c = *p;

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
/// @brief validate an index id (collection name + / + index id)
////////////////////////////////////////////////////////////////////////////////

bool Index::validateId (char const* key,
                        size_t* split) {
  char const* p = key;
  char c = *p;

  // extract collection name

  if (! (c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))) {
    return false;
  }

  ++p;

  while (true) {
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

  // validate index id
  return validateId(p);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief index comparator, used by the coordinator to detect if two index
/// contents are the same
////////////////////////////////////////////////////////////////////////////////

bool Index::IndexComparator (TRI_json_t const* lhs,
                             TRI_json_t const* rhs) {
  TRI_json_t const* typeJson = TRI_LookupObjectJson(lhs, "type");
  TRI_ASSERT(TRI_IsStringJson(typeJson));

  // type must be identical
  if (! TRI_CheckSameValueJson(typeJson, TRI_LookupObjectJson(rhs, "type"))) {
    return false;
  }

  triagens::mvcc::Index::IndexType type = triagens::mvcc::Index::type(typeJson->_value._string.data);


  // unique must be identical if present
  TRI_json_t const* value = TRI_LookupObjectJson(lhs, "unique");

  if (TRI_IsBooleanJson(value)) {
    if (! TRI_CheckSameValueJson(value, TRI_LookupObjectJson(rhs, "unique"))) {
      return false;
    }
  }

  // sparse must be identical if present
  value = TRI_LookupObjectJson(lhs, "sparse");

  if (TRI_IsBooleanJson(value)) {
    if (! TRI_CheckSameValueJson(value, TRI_LookupObjectJson(rhs, "sparse"))) {
      return false;
    }
  }


  if (type == triagens::mvcc::Index::TRI_IDX_TYPE_GEO1_INDEX) {
    // geoJson must be identical if present
    value = TRI_LookupObjectJson(lhs, "geoJson");

    if (TRI_IsBooleanJson(value)) {
      if (! TRI_CheckSameValueJson(value, TRI_LookupObjectJson(rhs, "geoJson"))) {
        return false;
      }
    }
  }
  else if (type == triagens::mvcc::Index::TRI_IDX_TYPE_FULLTEXT_INDEX) {
    // minLength
    value = TRI_LookupObjectJson(lhs, "minLength");

    if (TRI_IsNumberJson(value)) {
      if (! TRI_CheckSameValueJson(value, TRI_LookupObjectJson(rhs, "minLength"))) {
        return false;
      }
    }
  }
  else if (type == triagens::mvcc::Index::TRI_IDX_TYPE_CAP_CONSTRAINT) {
    // size, byteSize
    value = TRI_LookupObjectJson(lhs, "size");

    if (TRI_IsNumberJson(value)) {
      if (! TRI_CheckSameValueJson(value, TRI_LookupObjectJson(rhs, "size"))) {
        return false;
      }
    }

    value = TRI_LookupObjectJson(lhs, "byteSize");

    if (TRI_IsNumberJson(value)) {
      if (! TRI_CheckSameValueJson(value, TRI_LookupObjectJson(rhs, "byteSize"))) {
        return false;
      }
    }
  }

  // other index types: fields must be identical if present
  value = TRI_LookupObjectJson(lhs, "fields");

  if (TRI_IsArrayJson(value)) {
    if (type == triagens::mvcc::Index::TRI_IDX_TYPE_HASH_INDEX) {
      // compare fields in arbitrary order
      TRI_json_t const* r = TRI_LookupObjectJson(rhs, "fields");

      if (! TRI_IsArrayJson(r) ||
          value->_value._objects._length != r->_value._objects._length) {
        return false;
      }

      for (size_t i = 0; i < value->_value._objects._length; ++i) {
        TRI_json_t const* v = TRI_LookupArrayJson(value, i);

        bool found = false;

        for (size_t j = 0; j < r->_value._objects._length; ++j) {
          if (TRI_CheckSameValueJson(v, TRI_LookupArrayJson(r, j))) {
            found = true;
            break;
          }
        }

        if (! found) {
          return false;
        }
      }
    }
    else {
      if (! TRI_CheckSameValueJson(value, TRI_LookupObjectJson(rhs, "fields"))) {
        return false;
      }
    }

  }

  return true;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
