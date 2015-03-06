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
#include "Basics/ReadLocker.h"
#include "Basics/WriteLocker.h"
#include "Utils/Exception.h"
#include "VocBase/document-collection.h"
#include "VocBase/voc-shaper.h"

using namespace triagens::basics;
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
                      bool geoJson)
  : Index(id, fields),
    _collection(collection),
    _lock(),
    _paths(paths),
    _geoIndex(nullptr),
    _variant(geoJson ? INDEX_GEO_COMBINED_LAT_LON : INDEX_GEO_COMBINED_LON_LAT),
    _location(paths[0]),
    _latitude(0),
    _longitude(0),
    _geoJson(geoJson) {
 
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
                      std::vector<TRI_shape_pid_t> const& paths) 
  : Index(id, fields),
    _collection(collection),
    _lock(),
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
/// @brief query documents near a coordinate
////////////////////////////////////////////////////////////////////////////////

std::vector<std::pair<TRI_doc_mptr_t*, double>>* GeoIndex2::near (Transaction* transaction,
                                                                  double latitude,
                                                                  double longitude,
                                                                  size_t limit) {
  
  std::unique_ptr<std::vector<std::pair<TRI_doc_mptr_t*, double>>> theResult
        (new std::vector<std::pair<TRI_doc_mptr_t*, double>>);
  
  GeoCoordinate coordinate;
  coordinate.latitude  = latitude;
  coordinate.longitude = longitude;
 
  GeoCoordinates* result = nullptr;
  { 
    READ_LOCKER(_lock);
    result = GeoIndex_NearestCountPoints(transaction, _geoIndex, &coordinate, (int) limit);
  }

  if (result == nullptr) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
  }
 
  size_t const n = result->length;

  if (n == 0) {
    // empty result
    GeoIndex_CoordinatesFree(result);
    return theResult.release();
  }

  try {
    theResult->reserve(n);
    
    for (size_t i = 0; i < n; ++i) {
      theResult->emplace_back(std::make_pair(static_cast<TRI_doc_mptr_t*>(result->coordinates[i].data), result->distances[i])); 
    }
  }
  catch (...) {
    GeoIndex_CoordinatesFree(result);
    throw;
  }
    
  GeoIndex_CoordinatesFree(result);

  // sort result by distance
  std::sort(theResult->begin(), theResult->end(), [] (std::pair<TRI_doc_mptr_t const*, double> const& left, std::pair<TRI_doc_mptr_t const*, double> const& right) {
    return left.second < right.second;
  });

  return theResult.release();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief query documents within a radius
////////////////////////////////////////////////////////////////////////////////

std::vector<std::pair<TRI_doc_mptr_t*, double>>* GeoIndex2::within (Transaction* transaction,
                                                                    double latitude,
                                                                    double longitude,
                                                                    double radius) {
  
  std::unique_ptr<std::vector<std::pair<TRI_doc_mptr_t*, double>>> theResult
        (new std::vector<std::pair<TRI_doc_mptr_t*, double>>);
  
  GeoCoordinate coordinate;
  coordinate.latitude  = latitude;
  coordinate.longitude = longitude;
  
  GeoCoordinates* result = nullptr;
  {
    READ_LOCKER(_lock);
    result = GeoIndex_PointsWithinRadius(transaction, _geoIndex, &coordinate, radius);
  }

  if (result == nullptr) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
  }
 
  size_t const n = result->length;

  if (n == 0) {
    // empty result
    GeoIndex_CoordinatesFree(result);
    return theResult.release();
  }

  try {
    theResult->reserve(n);
    
    for (size_t i = 0; i < n; ++i) {
      theResult->emplace_back(std::make_pair(static_cast<TRI_doc_mptr_t*>(result->coordinates[i].data), result->distances[i])); 
    }
  }
  catch (...) {
    GeoIndex_CoordinatesFree(result);
    throw;
  }
    
  GeoIndex_CoordinatesFree(result);

  // sort result by distance
  std::sort(theResult->begin(), theResult->end(), [] (std::pair<TRI_doc_mptr_t const*, double> const& left, std::pair<TRI_doc_mptr_t const*, double> const& right) {
    return left.second < right.second;
  });

  return theResult.release();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief insert a document when loading a collection
////////////////////////////////////////////////////////////////////////////////

void GeoIndex2::insert (TRI_doc_mptr_t* doc) {
  insert(nullptr, nullptr, doc);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief insert document into index
////////////////////////////////////////////////////////////////////////////////

void GeoIndex2::insert (TransactionCollection*,
                        Transaction*,
                        TRI_doc_mptr_t* doc) {
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
    // invalid location values. simply ignore them
    return; 
  }

  // and insert into index
  GeoCoordinate gc;
  gc.latitude = latitude;
  gc.longitude = longitude;
  gc.data = static_cast<void*>(doc);

  WRITE_LOCKER(_lock);
  int res = GeoIndex_insert(_geoIndex, &gc);

  if (res < 0) {
    if (res == -1) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_INTERNAL);
    }
    if (res == -2) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
    }
    if (res == -3) {
      // location values out of bounds. simply ignore them
      return;
    }

    THROW_ARANGO_EXCEPTION(TRI_ERROR_INTERNAL);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief remove document from index
////////////////////////////////////////////////////////////////////////////////

TRI_doc_mptr_t* GeoIndex2::remove (TransactionCollection*,
                                   Transaction*,
                                   std::string const&,
                                   TRI_doc_mptr_t* doc) {
  return nullptr;  
}

////////////////////////////////////////////////////////////////////////////////
/// @brief forget document in index
////////////////////////////////////////////////////////////////////////////////

void GeoIndex2::forget (TransactionCollection*,
                        Transaction*,
                        TRI_doc_mptr_t* doc) {
  bool ok;
  bool missing;
  double latitude;
  double longitude;
  
  TRI_shaper_t* shaper = _collection->getShaper();  // ONLY IN INDEX, PROTECTED by RUNTIME

  // lookup latitude and longitude
  TRI_shaped_json_t shapedJson;
  TRI_EXTRACT_SHAPED_JSON_MARKER(shapedJson, doc->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

  // lookup OLD latitude and longitude
  if (_location != 0) {
    ok = extractDoubleList(shaper, &shapedJson, _location, &latitude, &longitude, &missing);
  }
  else {
    ok = extractDoubleArray(shaper, &shapedJson, _latitude, &latitude, &missing);
    ok = ok && extractDoubleArray(shaper, &shapedJson, _longitude, &longitude, &missing);
  }

  // ignore non-existing elements in geo-index
  // and remove old entry
  if (ok) {
    GeoCoordinate gc;
    gc.latitude = latitude;
    gc.longitude = longitude;
    gc.data = static_cast<void*>(doc);
  
    WRITE_LOCKER(_lock);
    GeoIndex_remove(_geoIndex, &gc);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief post insert (does nothing) 
////////////////////////////////////////////////////////////////////////////////

void GeoIndex2::preCommit (TransactionCollection*,
                           Transaction*) {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the amount of memory used by the index
////////////////////////////////////////////////////////////////////////////////

size_t GeoIndex2::memory () {
  READ_LOCKER(_lock);
  return GeoIndex_MemoryUsage(_geoIndex);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return a JSON representation of the index
////////////////////////////////////////////////////////////////////////////////

Json GeoIndex2::toJson (TRI_memory_zone_t* zone) const {
  std::vector<std::string> f;

  auto shaper = _collection->getShaper();

  if (_variant == INDEX_GEO_COMBINED_LAT_LON || 
      _variant == INDEX_GEO_COMBINED_LON_LAT) {
    // index has one field
  
    // convert location to string
    char const* location = TRI_AttributeNameShapePid(shaper, _location);

    if (location == nullptr) {
      return Json();
    }

    f.push_back(location);
  }
  else {
    // index has two fields
    char const* latitude = TRI_AttributeNameShapePid(shaper, _latitude);

    if (latitude == nullptr) {
      return Json();
    }

    f.push_back(latitude);

    char const* longitude = TRI_AttributeNameShapePid(shaper, _longitude);

    if (longitude == nullptr) {
      return Json();
    }
    
    f.push_back(longitude);
  }

  // create json
  Json json = Index::toJson(zone);

  if (_variant == INDEX_GEO_COMBINED_LAT_LON || 
      _variant == INDEX_GEO_COMBINED_LON_LAT) {
    json("geoJson", Json(zone, _geoJson));
  }

  // geo indexes are always non-unique
  // geo indexes are always sparse. 
  // "ignoreNull" has the same meaning as "sparse" and is only returned for backwards compatibility
  // the "constraint" attribute has no meaning since ArangoDB 2.5 and is only returned for backwards compatibility
  json("constraint", Json(zone, false))
      ("unique", Json(zone, false))
      ("ignoreNull", Json(zone, true))
      ("sparse", Json(zone, true));

  Json fields(zone, Json::Object, f.size());

  for (auto const& field : f) {
    fields.add(Json(zone, field));
  }
  json("fields", fields);

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

  if (json._sid == BasicShapes::TRI_SHAPE_SID_NUMBER) {
    *result = * (double*) json._data.data;
    return true;
  }
  if (json._sid == BasicShapes::TRI_SHAPE_SID_NULL) {
    *missing = true;
    return false;
  }

  return false;
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

    if (! ok || entry._sid != BasicShapes::TRI_SHAPE_SID_NUMBER) {
      return false;
    }

    *latitude = * (double*) entry._data.data;

    // longitude
    ok = TRI_AtListShapedJson((TRI_list_shape_t const*) shape, &list, 1, &entry);

    if (! ok || entry._sid != BasicShapes::TRI_SHAPE_SID_NUMBER) {
      return false;
    }

    *longitude = * (double*) entry._data.data;

    return true;
  }

  // homogenous list
  else if (shape->_type == TRI_SHAPE_HOMOGENEOUS_LIST) {
    TRI_homogeneous_list_shape_t const* hom = (TRI_homogeneous_list_shape_t const*) shape;

    if (hom->_sidEntry != BasicShapes::TRI_SHAPE_SID_NUMBER) {
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

    if (hom->_sidEntry != BasicShapes::TRI_SHAPE_SID_NUMBER) {
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
