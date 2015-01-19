////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC geo index 
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

#include "GeoIndex2.h"
#include "Utils/Exception.h"
#include "VocBase/document-collection.h"
#include "VocBase/voc-shaper.h"

using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                                    class GeoIndex
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new geo index, type "geo1"
////////////////////////////////////////////////////////////////////////////////

GeoIndex2::GeoIndex2 (TRI_idx_iid_t id,
                      TRI_document_collection_t* collection,
                      std::vector<std::string> const& fields,
                      std::vector<TRI_shape_pid_t> const& paths,
                      bool unique,
                      bool ignoreNull,
                      bool geoJson)
  : Index(id, collection, fields),
    _paths(paths),
    _geoIndex(nullptr),
    _variant(geoJson ? INDEX_GEO_COMBINED_LAT_LON : INDEX_GEO_COMBINED_LON_LAT),
    _location(paths[0]),
    _latitude(0),
    _longitude(0),
    _geoJson(geoJson),
    _unique(unique),
    _ignoreNull(ignoreNull) {
 
  _geoIndex = GeoIndex_new();

  if (_geoIndex == nullptr) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new geo index, type "geo2"
////////////////////////////////////////////////////////////////////////////////

GeoIndex2::GeoIndex2 (TRI_idx_iid_t id,
                      TRI_document_collection_t* collection,
                      std::vector<std::string> const& fields,
                      std::vector<TRI_shape_pid_t> const& paths,
                      bool unique,
                      bool ignoreNull)
  : Index(id, collection, fields, unique, ignoreNull, false),
    _paths(paths),
    _geoIndex(nullptr),
    _variant(INDEX_GEO_INDIVIDUAL_LAT_LON),
    _location(0),
    _latitude(paths[0]),
    _longitude(paths[1]),
    _geoJson(false) {
  
  _geoIndex = GeoIndex_new();

  if (_geoIndex == nullptr) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the geo index
////////////////////////////////////////////////////////////////////////////////

GeoIndex2::~GeoIndex2 () {
  if (_geoIndex != nullptr) {
    GeoIndex_free(_geoIndex);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief insert document into index
////////////////////////////////////////////////////////////////////////////////

void GeoIndex2::insert (TransactionCollection*,
                        TRI_doc_mptr_t const* doc) {
  TRI_shaper_t* shaper = _collection->getShaper();  // ONLY IN INDEX, PROTECTED by RUNTIME

  // lookup latitude and longitude
  TRI_shaped_json_t shapedJson;
  TRI_EXTRACT_SHAPED_JSON_MARKER(shapedJson, doc->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

  bool ok;
  bool missing;
  double latitude;
  double longitude;

  if (_location != 0) {
    if (_geoJson) {
      ok = extractDoubleList(shaper, &shapedJson, _location, &longitude, &latitude, &missing);
    }
    else {
      ok = extractDoubleList(shaper, &shapedJson, _location, &latitude, &longitude, &missing);
    }
  }
  else {
    ok = extractDoubleArray(shaper, &shapedJson, _latitude, &latitude, &missing);
    ok = ok && extractDoubleArray(shaper, &shapedJson, _longitude, &longitude, &missing);
  }

  if (! ok) {
    if (_unique) {
      if (_ignoreNull && missing) {
        return;
      }
      else {
        THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_GEO_INDEX_VIOLATED);
      }
    }
    else {
      return;
    }
  }

  // and insert into index
  GeoCoordinate gc;
  gc.latitude = latitude;
  gc.longitude = longitude;

  gc.data = CONST_CAST(doc);

  int res = GeoIndex_insert(_geoIndex, &gc);

  if (res == -1) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_INTERNAL);
  }
  else if (res == -2) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
  }
  else if (res == -3) {
    if (_unique) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_GEO_INDEX_VIOLATED);
    }
    else {
      return;
    }
  }
  else if (res < 0) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_INTERNAL);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief remove document from index
////////////////////////////////////////////////////////////////////////////////

void GeoIndex2::remove (TransactionCollection*,
                        TRI_doc_mptr_t const* doc) {
  TRI_shaper_t* shaper = _collection->getShaper();  // ONLY IN INDEX, PROTECTED by RUNTIME

  TRI_shaped_json_t shapedJson;
  TRI_EXTRACT_SHAPED_JSON_MARKER(shapedJson, doc->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

  // lookup OLD latitude and longitude
  bool ok;
  double latitude;
  double longitude;
  bool missing;

  if (_location != 0) {
    ok = extractDoubleList(shaper, &shapedJson, _location, &latitude, &longitude, &missing);
  }
  else {
    ok = extractDoubleArray(shaper, &shapedJson, _latitude, &latitude, &missing);
    ok = ok && extractDoubleArray(shaper, &shapedJson, _longitude, &longitude, &missing);
  }

  // and remove old entry
  if (ok) {
    GeoCoordinate gc;
    gc.latitude = latitude;
    gc.longitude = longitude;

    gc.data = CONST_CAST(doc);

    // ignore non-existing elements in geo-index
    GeoIndex_remove(_geoIndex, &gc);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief post insert (does nothing) 
////////////////////////////////////////////////////////////////////////////////

void GeoIndex2::preCommit (TransactionCollection*) {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the amount of memory used by the index
////////////////////////////////////////////////////////////////////////////////

size_t GeoIndex2::memory () {
  return GeoIndex_MemoryUsage(_geoIndex);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return a JSON representation of the index
////////////////////////////////////////////////////////////////////////////////

GeoIndex2::toJson (TRI_memory_zone_t* zone) const {
  std::vector<std::string> f;

  auto shaper = _collection->getShaper();

  if (_variant == INDEX_GEO_COMBINED_LAT_LON || 
      _variant == INDEX_GEO_COMBINED_LON_LAT) {
    // index has one field
  
    // convert location to string
    TRI_shape_path_t const* path = shaper->lookupAttributePathByPid(shaper, _location);  // ONLY IN INDEX, PROTECTED by RUNTIME

    if (path == nullptr) {
      return nullptr;
    }

    char const* location = TRI_NAME_SHAPE_PATH(path);
    f.push_back(location);
  }
  else {
    // index has two fields
    TRI_shape_path_t const* path = shaper->lookupAttributePathByPid(shaper, _latitude);  // ONLY IN INDEX, PROTECTED by RUNTIME

    if (path == nullptr) {
      return nullptr;
    }

    char const* latitude = TRI_NAME_SHAPE_PATH(path);
    f.push_back(latitude);

    // convert longitude to string
    path = shaper->lookupAttributePathByPid(shaper, _longitude);  // ONLY IN INDEX, PROTECTED by RUNTIME

    if (path == nullptr) {
      return nullptr;
    }

    char const* longitude = TRI_NAME_SHAPE_PATH(path);
    f.push_back(longitude);
  }

  // create json
  TRI_json_t* json = Index::toJson(zone);

  if (json != nullptr) {
    if (_variant == INDEX_GEO_COMBINED_LAT_LON || 
        _variant == INDEX_GEO_COMBINED_LON_LAT) {
      TRI_Insert3ObjectJson(zone, json, "geoJson", TRI_CreateBooleanJson(zone, _geoJson));
    }

    // "constraint" and "unique" are identical for geo indexes.
    // we return "constraint" just for downwards-compatibility
    TRI_Insert3ObjectJson(zone, json, "constraint", TRI_CreateBooleanJson(zone, _unique));
    TRI_Insert3ObjectJson(zone, json, "ignoreNull", TRI_CreateBooleanJson(zone, _ignoreNull));

    TRI_json_t* fields = TRI_CreateArrayJson(zone, f.size());

    if (fields != nullptr) {
      for (auto field : f) {
        TRI_PushBack3ArrayJson(zone, fields, TRI_CreateStringCopyJson(zone, field.c_str(), field.size()));
      }
      TRI_Insert3ObjectJson(zone, json, "fields", fields);
    }
  }

  return json;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief extracts a double value from an array
////////////////////////////////////////////////////////////////////////////////

bool GeoIndex2::extractDoubleArray (TRI_shaper_t* shaper,
                                    TRI_shaped_json_t const* document,
                                    TRI_shape_pid_t pid,
                                    double* result,
                                    bool* missing) {
  *missing = false;

  TRI_shape_t const* shape;
  TRI_shaped_json_t json;

  bool ok = TRI_ExtractShapedJsonVocShaper(shaper, document, 0, pid, &json, &shape);

  if (! ok) {
    return false;
  }

  if (shape == nullptr) {
    *missing = true;
    return false;
  }
  else if (json._sid == TRI_LookupBasicSidShaper(TRI_SHAPE_NUMBER)) {
    *result = * (double*) json._data.data;
    return true;
  }
  else if (json._sid == TRI_LookupBasicSidShaper(TRI_SHAPE_NULL)) {
    *missing = true;
    return false;
  }
  else {
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief extracts a double value from a list
////////////////////////////////////////////////////////////////////////////////

bool GeoIndex2::extractDoubleList (TRI_shaper_t* shaper,
                                   TRI_shaped_json_t const* document,
                                   TRI_shape_pid_t pid,
                                   double* latitude,
                                   double* longitude,
                                   bool* missing) {
  TRI_shaped_json_t entry;

  *missing = false;

  TRI_shape_t const* shape;
  TRI_shaped_json_t list;

  bool ok = TRI_ExtractShapedJsonVocShaper(shaper, document, 0, pid, &list, &shape);

  if (! ok) {
    return false;
  }

  if (shape == nullptr) {
    *missing = true;
    return false;
  }

  // in-homogenous list
  if (shape->_type == TRI_SHAPE_LIST) {
    size_t len = TRI_LengthListShapedJson((TRI_list_shape_t const*) shape, &list);

    if (len < 2) {
      return false;
    }

    // latitude
    ok = TRI_AtListShapedJson((TRI_list_shape_t const*) shape, &list, 0, &entry);

    if (! ok || entry._sid != TRI_LookupBasicSidShaper(TRI_SHAPE_NUMBER)) {
      return false;
    }

    *latitude = * (double*) entry._data.data;

    // longitude
    ok = TRI_AtListShapedJson((TRI_list_shape_t const*) shape, &list, 1, &entry);

    if (! ok || entry._sid != TRI_LookupBasicSidShaper(TRI_SHAPE_NUMBER)) {
      return false;
    }

    *longitude = * (double*) entry._data.data;

    return true;
  }

  // homogenous list
  else if (shape->_type == TRI_SHAPE_HOMOGENEOUS_LIST) {
    TRI_homogeneous_list_shape_t const* hom = (TRI_homogeneous_list_shape_t const*) shape;

    if (hom->_sidEntry != TRI_LookupBasicSidShaper(TRI_SHAPE_NUMBER)) {
      return false;
    }

    size_t len = TRI_LengthHomogeneousListShapedJson((TRI_homogeneous_list_shape_t const*) shape, &list);

    if (len < 2) {
      return false;
    }

    // latitude
    ok = TRI_AtHomogeneousListShapedJson((TRI_homogeneous_list_shape_t const*) shape, &list, 0, &entry);

    if (! ok) {
      return false;
    }

    *latitude = * (double*) entry._data.data;

    // longitude
    ok = TRI_AtHomogeneousListShapedJson((TRI_homogeneous_list_shape_t const*) shape, &list, 1, &entry);

    if (! ok) {
      return false;
    }

    *longitude = * (double*) entry._data.data;

    return true;
  }

  // homogeneous list
  else if (shape->_type == TRI_SHAPE_HOMOGENEOUS_SIZED_LIST) {
    TRI_homogeneous_sized_list_shape_t const* hom = (TRI_homogeneous_sized_list_shape_t const*) shape;

    if (hom->_sidEntry != TRI_LookupBasicSidShaper(TRI_SHAPE_NUMBER)) {
      return false;
    }

    size_t len = TRI_LengthHomogeneousSizedListShapedJson((TRI_homogeneous_sized_list_shape_t const*) shape, &list);

    if (len < 2) {
      return false;
    }

    // latitude
    ok = TRI_AtHomogeneousSizedListShapedJson((TRI_homogeneous_sized_list_shape_t const*) shape, &list, 0, &entry);

    if (! ok) {
      return false;
    }

    *latitude = * (double*) entry._data.data;

    // longitude
    ok = TRI_AtHomogeneousSizedListShapedJson((TRI_homogeneous_sized_list_shape_t const*) shape, &list, 1, &entry);

    if (! ok) {
      return false;
    }

    *longitude = * (double*) entry._data.data;

    return true;
  }

  // null
  else if (shape->_type == TRI_SHAPE_NULL) {
    *missing = true;
  }

  // ups
  return false;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
