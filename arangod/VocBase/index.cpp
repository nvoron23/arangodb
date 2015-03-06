////////////////////////////////////////////////////////////////////////////////
/// @brief index
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

#include "index.h"
#include "Basics/files.h"
#include "Basics/json.h"
#include "Basics/logging.h"
#include "Basics/json-utilities.h"
#include "Mvcc/Index.h"
#include "Utils/Exception.h"
#include "VocBase/document-collection.h"
#include "Wal/LogfileManager.h"
#include "Wal/Marker.h"

// -----------------------------------------------------------------------------
// --SECTION--                                                             INDEX
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief removes an index file
////////////////////////////////////////////////////////////////////////////////

static std::string BuildIndexFilename (char const* directory,
                                       TRI_idx_iid_t iid) {
  std::string filename;
  filename.append(directory);
  filename.append(TRI_DIR_SEPARATOR_STR);
  filename.append("index-");
  filename.append(std::to_string(iid));
  filename.append(".json");

  return filename;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief removes an index file
////////////////////////////////////////////////////////////////////////////////

int TRI_RemoveIndexFile (TRI_document_collection_t* collection, 
                         TRI_idx_iid_t iid) {
  std::string&& filename = BuildIndexFilename(collection->_directory, iid);

  int res = TRI_UnlinkFile(filename.c_str());

  if (res != TRI_ERROR_NO_ERROR) {
    LOG_ERROR("cannot remove index definition: %s", TRI_last_error());
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief saves an index
////////////////////////////////////////////////////////////////////////////////

int TRI_SaveIndex (TRI_document_collection_t* document,
                   triagens::mvcc::Index const* index,
                   bool writeMarker) {
  // convert into JSON
  auto json = index->toJson(TRI_UNKNOWN_MEM_ZONE);

  if (json.json() == nullptr) {
    LOG_TRACE("cannot save index definition: index cannot be jsonified");
    return TRI_set_errno(TRI_ERROR_INTERNAL);
  }

  std::string&& filename = BuildIndexFilename(document->_directory, index->id());

  TRI_vocbase_t* vocbase = document->_vocbase;

  // and save
  bool ok = TRI_SaveJson(filename.c_str(), json.json(), document->_vocbase->_settings.forceSyncProperties);

  if (! ok) {
    LOG_ERROR("cannot save index definition: %s", TRI_last_error());
    return TRI_errno();
  }

  int res = TRI_ERROR_NO_ERROR;

  if (writeMarker) {
    try {
      triagens::wal::CreateIndexMarker marker(vocbase->_id, document->_info._cid, index->id(), triagens::basics::JsonHelper::toString(json.json()));
      triagens::wal::SlotInfoCopy slotInfo = triagens::wal::LogfileManager::instance()->allocateAndWrite(marker, false);

      if (slotInfo.errorCode != TRI_ERROR_NO_ERROR) {
        THROW_ARANGO_EXCEPTION(slotInfo.errorCode);
      }
    }
    catch (triagens::arango::Exception const& ex) {
      res = ex.code();
    }
    catch (...) {
      res = TRI_ERROR_INTERNAL;
    }
  }

  // TODO: what to do here in case of error?
  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief index comparator, used by the coordinator to detect if two index
/// contents are the same
////////////////////////////////////////////////////////////////////////////////

bool IndexComparator (TRI_json_t const* lhs,
                      TRI_json_t const* rhs) {
  TRI_json_t const* typeJson = TRI_LookupObjectJson(lhs, "type");
  TRI_ASSERT(TRI_IsStringJson(typeJson));

  // type must be identical
  if (! TRI_CheckSameValueJson(typeJson, TRI_LookupObjectJson(rhs, "type"))) {
    return false;
  }

  TRI_idx_type_e type = triagens::mvcc::Index::type(typeJson->_value._string.data);


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


  if (type == TRI_IDX_TYPE_GEO1_INDEX) {
    // geoJson must be identical if present
    value = TRI_LookupObjectJson(lhs, "geoJson");

    if (TRI_IsBooleanJson(value)) {
      if (! TRI_CheckSameValueJson(value, TRI_LookupObjectJson(rhs, "geoJson"))) {
        return false;
      }
    }
  }
  else if (type == TRI_IDX_TYPE_FULLTEXT_INDEX) {
    // minLength
    value = TRI_LookupObjectJson(lhs, "minLength");

    if (TRI_IsNumberJson(value)) {
      if (! TRI_CheckSameValueJson(value, TRI_LookupObjectJson(rhs, "minLength"))) {
        return false;
      }
    }
  }
  else if (type == TRI_IDX_TYPE_CAP_CONSTRAINT) {
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
    if (type == TRI_IDX_TYPE_HASH_INDEX) {
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
