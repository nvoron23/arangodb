////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC skiplist index
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

#include "SkiplistIndex2.h"
#include "Utils/Exception.h"
#include "VocBase/document-collection.h"
#include "VocBase/voc-shaper.h"

using namespace triagens::basics;
using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                               class SkiplistIndex
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new skiplist index
////////////////////////////////////////////////////////////////////////////////

SkiplistIndex2::SkiplistIndex2 (TRI_idx_iid_t id,
                                TRI_document_collection_t* collection,
                                std::vector<std::string> const& fields,
                                std::vector<TRI_shape_pid_t> const& paths,
                                bool unique,
                                bool ignoreNull,
                                bool sparse)
  : Index(id, collection, fields),
    _paths(paths),
    _skiplistIndex(nullptr),
    _unique(unique),
    _ignoreNull(ignoreNull),
    _sparse(sparse) {
  
    _skiplistIndex = SkiplistIndex_new(_collection, _paths.size(), unique);

    if (_skiplistIndex == nullptr) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
    }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the skiplist index
////////////////////////////////////////////////////////////////////////////////

SkiplistIndex2::~SkiplistIndex2 () {
  if (_skiplistIndex != nullptr) {
    SkiplistIndex_free(_skiplistIndex);
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief insert document into index
////////////////////////////////////////////////////////////////////////////////
        
void SkiplistIndex2::insert (TransactionCollection*,
                             TRI_doc_mptr_t* doc) {
  // ...........................................................................
  // Allocate storage to shaped json objects stored as a simple list.
  // These will be used for comparisions
  // ...........................................................................

  TRI_skiplist_index_element_t skiplistElement;
  skiplistElement._subObjects = static_cast<TRI_shaped_sub_t*>(TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, sizeof(TRI_shaped_sub_t) * _paths.size(), false));

  if (skiplistElement._subObjects == nullptr) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
  }

  int res = shapify(&skiplistElement, doc);
  // ...........................................................................
  // most likely the cause of this error is that the index is sparse
  // and not all attributes the index needs are set -- so the document
  // is ignored. So not really an error at all. Note that this does
  // not happen in a non-sparse skiplist index, in which empty
  // attributes are always treated as if they were bound to null, so
  // TRI_ERROR_ARANGO_INDEX_DOCUMENT_ATTRIBUTE_MISSING cannot happen at
  // all.
  // ...........................................................................

  if (res != TRI_ERROR_NO_ERROR) {

    // ..........................................................................
    // Deallocated the memory already allocated to skiplistElement.fields
    // ..........................................................................

    TRI_Free(TRI_UNKNOWN_MEM_ZONE, skiplistElement._subObjects);

    // .........................................................................
    // It may happen that the document does not have the necessary
    // attributes to be included within the hash index, in this case do
    // not report back an error.
    // .........................................................................

    if (res == TRI_ERROR_ARANGO_INDEX_DOCUMENT_ATTRIBUTE_MISSING) {
      return;
    }

    THROW_ARANGO_EXCEPTION(res);
  }

  // ...........................................................................
  // Fill the json field list from the document for skiplist index
  // ...........................................................................

  res = SkiplistIndex_insert(_skiplistIndex, &skiplistElement);

  // ...........................................................................
  // Memory which has been allocated to skiplistElement.fields remains allocated
  // contents of which are stored in the hash array.
  // ...........................................................................

  TRI_Free(TRI_UNKNOWN_MEM_ZONE, skiplistElement._subObjects);

  if (res != TRI_ERROR_NO_ERROR) {
    THROW_ARANGO_EXCEPTION(res);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief remove document from index
////////////////////////////////////////////////////////////////////////////////
        
TRI_doc_mptr_t* SkiplistIndex2::remove (TransactionCollection*,
                                        std::string const&,
                                        TRI_doc_mptr_t const* doc) {
  // ...........................................................................
  // Allocate some memory for the SkiplistIndexElement structure
  // ...........................................................................

  TRI_skiplist_index_element_t skiplistElement;
  skiplistElement._subObjects = static_cast<TRI_shaped_sub_t*>(TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, sizeof(TRI_shaped_sub_t) * _paths.size(), false));

  if (skiplistElement._subObjects == nullptr) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
  }

  // ..........................................................................
  // Fill the json field list from the document
  // ..........................................................................

  int res = shapify(&skiplistElement, doc);

  // ..........................................................................
  // Error returned generally implies that the document never was part of the
  // skiplist index
  // ..........................................................................

  if (res != TRI_ERROR_NO_ERROR) {

    // ........................................................................
    // Deallocate memory allocated to skiplistElement.fields above
    // ........................................................................

    TRI_Free(TRI_UNKNOWN_MEM_ZONE, skiplistElement._subObjects);

    // ........................................................................
    // It may happen that the document does not have the necessary attributes
    // to have particpated within the hash index. In this case, we do not
    // report an error to the calling procedure.
    // ........................................................................

    if (res == TRI_ERROR_ARANGO_INDEX_DOCUMENT_ATTRIBUTE_MISSING) {
      return nullptr;   // this is nonsense
    }

    THROW_ARANGO_EXCEPTION(res);
  }

  // ...........................................................................
  // Attempt the removal for skiplist indexes
  // ...........................................................................

  res = SkiplistIndex_remove(_skiplistIndex, &skiplistElement);

  // ...........................................................................
  // Deallocate memory allocated to skiplistElement.fields above
  // ...........................................................................

  TRI_Free(TRI_UNKNOWN_MEM_ZONE, skiplistElement._subObjects);

  if (res != TRI_ERROR_NO_ERROR) {
    THROW_ARANGO_EXCEPTION(res);
  }
  return nullptr;   // this is nonsense
}

////////////////////////////////////////////////////////////////////////////////
/// @brief forget document in the index
////////////////////////////////////////////////////////////////////////////////
        
void SkiplistIndex2::forget (TransactionCollection*,
                             TRI_doc_mptr_t const* doc) {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief post-insert (does nothing)
////////////////////////////////////////////////////////////////////////////////

void SkiplistIndex2::preCommit (TransactionCollection*) {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief determine the amount of memory used by the index
////////////////////////////////////////////////////////////////////////////////

size_t SkiplistIndex2::memory () {
  return SkiplistIndex_memoryUsage(_skiplistIndex);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return a JSON representation of the index
////////////////////////////////////////////////////////////////////////////////
        
Json SkiplistIndex2::toJson (TRI_memory_zone_t* zone) const {
  Json json = Index::toJson(zone);
  Json fields(zone, Json::Array, _fields.size());
  for (auto& field : _fields) {
    fields.add(Json(zone, field));
  }
  json("fields", fields)
      ("unique", Json(zone, _unique))
      ("ignoreNull", Json(zone, _ignoreNull))
      ("sparse", Json(zone, _sparse));
  return json;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                 private functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief helper for skiplist methods
////////////////////////////////////////////////////////////////////////////////

int SkiplistIndex2::shapify (TRI_skiplist_index_element_t* skiplistElement,
                             TRI_doc_mptr_t const* doc) {
  // ..........................................................................
  // Assign the document to the SkiplistIndexElement structure so that it can
  // be retrieved later.
  // ..........................................................................

  TRI_ASSERT(doc != nullptr);
  TRI_ASSERT(doc->getDataPtr() != nullptr);   // ONLY IN INDEX, PROTECTED by RUNTIME

  TRI_shaped_json_t shapedJson;
  TRI_EXTRACT_SHAPED_JSON_MARKER(shapedJson, doc->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

  if (shapedJson._sid == TRI_SHAPE_ILLEGAL) {
    return TRI_ERROR_INTERNAL;
  }

  skiplistElement->_document = const_cast<TRI_doc_mptr_t*>(doc);
  char const* ptr = skiplistElement->_document->getShapedJsonPtr();  // ONLY IN INDEX, PROTECTED by RUNTIME

  size_t const n = _paths.size();

  for (size_t i = 0; i < n; ++i) {
    TRI_shape_pid_t path = _paths[i];

    // ..........................................................................
    // Determine if document has that particular shape
    // ..........................................................................

    TRI_shape_access_t const* acc = TRI_FindAccessorVocShaper(_collection->getShaper(), shapedJson._sid, path);  // ONLY IN INDEX, PROTECTED by RUNTIME

    if (acc == nullptr || acc->_resultSid == TRI_SHAPE_ILLEGAL) {
      // OK, the document does not contain the attributed needed by 
      // the index, are we sparse?
      if (! _sparse) {
        // No, so let's fake a JSON null:
        skiplistElement->_subObjects[i]._sid = TRI_LookupBasicSidShaper(TRI_SHAPE_NULL);
        skiplistElement->_subObjects[i]._length = 0;
        skiplistElement->_subObjects[i]._offset = 0;
        continue;
      }

      return TRI_ERROR_ARANGO_INDEX_DOCUMENT_ATTRIBUTE_MISSING;
    }

    // ..........................................................................
    // Extract the field
    // ..........................................................................

    TRI_shaped_json_t shapedObject;
    if (! TRI_ExecuteShapeAccessor(acc, &shapedJson, &shapedObject)) {
      return TRI_ERROR_INTERNAL;
    }

    // .........................................................................
    // Store the field
    // .........................................................................

    skiplistElement->_subObjects[i]._sid = shapedObject._sid;
    skiplistElement->_subObjects[i]._length = shapedObject._data.length;
    skiplistElement->_subObjects[i]._offset = static_cast<uint32_t>(((char const*) shapedObject._data.data) - ptr);
  }

  return TRI_ERROR_NO_ERROR;
}
        
// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
