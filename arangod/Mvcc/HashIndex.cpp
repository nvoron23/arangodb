////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC hash index
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

#include "HashIndex.h"
#include "HashIndex/hash-index.h"
#include "Utils/Exception.h"
#include "VocBase/document-collection.h"
#include "VocBase/voc-shaper.h"

using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                                   class HashIndex
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new hash index
////////////////////////////////////////////////////////////////////////////////

HashIndex::HashIndex (TRI_idx_iid_t id,
                      TRI_document_collection_t* collection,
                      std::vector<std::string> const& fields,
                      std::vector<TRI_shape_pid_t> const& paths,
                      bool unique) 
  : Index(id, collection, fields, unique, false, false),
    _paths(paths) {
  
  int res;
  if (unique) {
    res = TRI_InitHashArray(&_hashArray, _fields.size());
  }
  else {
    res = TRI_InitHashArrayMulti(&_hashArrayMulti, _fields.size());
  }

  if (res != TRI_ERROR_NO_ERROR) {
    THROW_ARANGO_EXCEPTION(res);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the hash index
////////////////////////////////////////////////////////////////////////////////

HashIndex::~HashIndex () {
  if (_unique) {
    TRI_DestroyHashArray(&_hashArray);
  }
  else {
    TRI_DestroyHashArrayMulti(&_hashArrayMulti);
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief insert a document into the index
////////////////////////////////////////////////////////////////////////////////
        
int HashIndex::insert (TRI_doc_mptr_t const* doc, 
                       bool isRollback) {
  int res;

  if (_unique) {
    TRI_hash_index_element_t hashElement;
    res = hashIndexHelperAllocate<TRI_hash_index_element_t>(&hashElement, doc);

    if (res == TRI_ERROR_ARANGO_INDEX_DOCUMENT_ATTRIBUTE_MISSING) {
      freeSubObjectsHashIndexElement<TRI_hash_index_element_t>(&hashElement);
      return TRI_ERROR_NO_ERROR;
    }

    if (res != TRI_ERROR_NO_ERROR) {
      freeSubObjectsHashIndexElement<TRI_hash_index_element_t>(&hashElement);
      return res;
    }

    res = insertUnique(&hashElement, isRollback);

    if (res != TRI_ERROR_NO_ERROR) {
      freeSubObjectsHashIndexElement<TRI_hash_index_element_t>(&hashElement);
    }
  }
  else {
    TRI_hash_index_element_multi_t hashElement;
    res = hashIndexHelperAllocate<TRI_hash_index_element_multi_t>(&hashElement, doc);

    if (res == TRI_ERROR_ARANGO_INDEX_DOCUMENT_ATTRIBUTE_MISSING) {
      freeSubObjectsHashIndexElement<TRI_hash_index_element_multi_t>(&hashElement);
      return TRI_ERROR_NO_ERROR;
    }

    if (res != TRI_ERROR_NO_ERROR) {
      freeSubObjectsHashIndexElement<TRI_hash_index_element_multi_t>(&hashElement);
      return res;
    }

    res = insertMulti(&hashElement, isRollback);
    if (res != TRI_ERROR_NO_ERROR) {
      freeSubObjectsHashIndexElement<TRI_hash_index_element_multi_t>(&hashElement);
    }
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief remove a document from the index
////////////////////////////////////////////////////////////////////////////////

int HashIndex::remove (TRI_doc_mptr_t const* doc,
                       bool isRollback) {
  int res;

  if (_unique) {
    TRI_hash_index_element_t hashElement;
    res = hashIndexHelperAllocate<TRI_hash_index_element_t>(&hashElement, doc);

    if (res == TRI_ERROR_ARANGO_INDEX_DOCUMENT_ATTRIBUTE_MISSING) {
      freeSubObjectsHashIndexElement<TRI_hash_index_element_t>(&hashElement);
      return TRI_ERROR_NO_ERROR;
    }

    if (res != TRI_ERROR_NO_ERROR) {
      freeSubObjectsHashIndexElement<TRI_hash_index_element_t>(&hashElement);
      return res;
    }
    res = removeUnique(&hashElement);
    freeSubObjectsHashIndexElement<TRI_hash_index_element_t>(&hashElement);
  }
  else {
    TRI_hash_index_element_multi_t hashElement;
    res = hashIndexHelperAllocate<TRI_hash_index_element_multi_t>(&hashElement, doc);

    if (res == TRI_ERROR_ARANGO_INDEX_DOCUMENT_ATTRIBUTE_MISSING) {
      freeSubObjectsHashIndexElement<TRI_hash_index_element_multi_t>(&hashElement);
      return TRI_ERROR_NO_ERROR;
    }

    if (res != TRI_ERROR_NO_ERROR) {
      freeSubObjectsHashIndexElement<TRI_hash_index_element_multi_t>(&hashElement);
      return res;
    }
    res = removeMulti(&hashElement);
    freeSubObjectsHashIndexElement<TRI_hash_index_element_multi_t>(&hashElement);
  }

  return res;
}        

////////////////////////////////////////////////////////////////////////////////
/// @brief post insert (does nothing)
////////////////////////////////////////////////////////////////////////////////
        
int HashIndex::postInsert (TRI_transaction_collection_t*, 
                           TRI_doc_mptr_t const*) {
  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief provides a hint for the expected index size
////////////////////////////////////////////////////////////////////////////////

int HashIndex::sizeHint (size_t size) {
  if (_unique) {
    TRI_ResizeHashArray(&_hashArray, size);
  }
  else {
    TRI_ResizeHashArrayMulti(&_hashArrayMulti, size);
  }

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the memory used by the index
////////////////////////////////////////////////////////////////////////////////
  
size_t HashIndex::memory () {
  if (_unique) {
    return static_cast<size_t>(keyEntrySize() * _hashArray._nrUsed + 
                               TRI_MemoryUsageHashArray(&_hashArray));
  }
  else {
    return static_cast<size_t>(keyEntrySize() * _hashArrayMulti._nrUsed + 
                               TRI_MemoryUsageHashArrayMulti(&_hashArrayMulti));
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return a JSON representation of the index
////////////////////////////////////////////////////////////////////////////////

TRI_json_t* HashIndex::toJson (TRI_memory_zone_t* zone) const {
  TRI_json_t* json = Index::toJson(zone);

  if (json != nullptr) {
    TRI_json_t* fields = TRI_CreateArrayJson(zone, _fields.size());

    if (fields != nullptr) {
      for (auto const& field : _fields) {
        TRI_PushBack3ArrayJson(zone, fields, TRI_CreateStringCopyJson(zone, field.c_str(), field.size()));
      }
      TRI_Insert3ObjectJson(zone, json, "fields", fields);
    }
  }

  return json;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the memory needed for an index key entry
////////////////////////////////////////////////////////////////////////////////

size_t HashIndex::keyEntrySize () const {
  return _fields.size() * sizeof(TRI_shaped_json_t);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief fills the index search from hash index element
////////////////////////////////////////////////////////////////////////////////

template<typename T>
int HashIndex::fillIndexSearchValueByHashIndexElement (TRI_index_search_value_t* key,
                                                       T const* element) {
  key->_values = static_cast<TRI_shaped_json_t*>(TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, keyEntrySize(), false));

  if (key->_values == nullptr) {
    return TRI_ERROR_OUT_OF_MEMORY;
  }

  char const* ptr = element->_document->getShapedJsonPtr();  // ONLY IN INDEX
  size_t const n = _fields.size();

  for (size_t i = 0;  i < n;  ++i) {
    key->_values[i]._sid         = element->_subObjects[i]._sid;
    key->_values[i]._data.length = element->_subObjects[i]._length;
    key->_values[i]._data.data   = const_cast<char*>(ptr + element->_subObjects[i]._offset);
  }

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief creates space for sub-objects in the hash index element
////////////////////////////////////////////////////////////////////////////////

template<typename T>
int HashIndex::allocateSubObjectsHashIndexElement (T* element) {

  TRI_ASSERT_EXPENSIVE(element->_subObjects == nullptr);
  element->_subObjects = static_cast<TRI_shaped_sub_t*>(TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, keyEntrySize(), false));

  if (element->_subObjects == nullptr) {
    return TRI_ERROR_OUT_OF_MEMORY;
  }

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief frees space for sub-objects in the hash index element
////////////////////////////////////////////////////////////////////////////////

template<typename T>
void HashIndex::freeSubObjectsHashIndexElement (T* element) {
  if (element->_subObjects != nullptr) {
    TRI_Free(TRI_UNKNOWN_MEM_ZONE, element->_subObjects);
    element->_subObjects = nullptr;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief helper for hashing
///
/// This function takes a document master pointer and creates a corresponding
/// hash index element. The index element contains the document master pointer
/// and a lists of offsets and sizes describing the parts of the documents to be
/// hashed and the shape identifier of each part.
////////////////////////////////////////////////////////////////////////////////

template<typename T>
int HashIndex::hashIndexHelper (T* hashElement,
                                TRI_doc_mptr_t const* doc) {
  // .............................................................................
  // Assign the document to the TRI_hash_index_element_t structure - so that it
  // can later be retreived.
  // .............................................................................

  TRI_shaped_json_t shapedJson;
  TRI_EXTRACT_SHAPED_JSON_MARKER(shapedJson, doc->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

  hashElement->_document = const_cast<TRI_doc_mptr_t*>(doc);
  char const* ptr = doc->getShapedJsonPtr();  // ONLY IN INDEX

  // .............................................................................
  // Extract the attribute values
  // .............................................................................

  TRI_shaped_sub_t shapedSub;           // the relative sub-object
  TRI_shaper_t* shaper = _collection->getShaper();  // ONLY IN INDEX, PROTECTED by RUNTIME
  int res = TRI_ERROR_NO_ERROR;

  size_t const n = _paths.size();

  for (size_t i = 0; i < n; ++i) {
    TRI_shape_pid_t path = _paths[i];

    // determine if document has that particular shape
    TRI_shape_access_t const* acc = TRI_FindAccessorVocShaper(shaper, shapedJson._sid, path);

    // field not part of the object
    if (acc == nullptr || acc->_resultSid == TRI_SHAPE_ILLEGAL) {
      shapedSub._sid    = TRI_LookupBasicSidShaper(TRI_SHAPE_NULL);
      shapedSub._length = 0;
      shapedSub._offset = 0;

      res = TRI_ERROR_ARANGO_INDEX_DOCUMENT_ATTRIBUTE_MISSING;
    }

    // extract the field
    else {
      TRI_shaped_json_t shapedObject;       // the sub-object

      if (! TRI_ExecuteShapeAccessor(acc, &shapedJson, &shapedObject)) {
        // hashElement->fields: memory deallocated in the calling procedure
        return TRI_ERROR_INTERNAL;
      }

      if (shapedObject._sid == TRI_LookupBasicSidShaper(TRI_SHAPE_NULL)) {
        res = TRI_ERROR_ARANGO_INDEX_DOCUMENT_ATTRIBUTE_MISSING;
      }

      shapedSub._sid    = shapedObject._sid;
      shapedSub._length = shapedObject._data.length;
      shapedSub._offset = static_cast<uint32_t>(((char const*) shapedObject._data.data) - ptr);
    }

    // store the json shaped sub-object -- this is what will be hashed
    hashElement->_subObjects[i] = shapedSub;
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief index helper for hashing with allocation
////////////////////////////////////////////////////////////////////////////////

template<typename T>
int HashIndex::hashIndexHelperAllocate (T* hashElement,
                                        TRI_doc_mptr_t const* document) {
  // .............................................................................
  // Allocate storage to shaped json objects stored as a simple list.  These
  // will be used for hashing.  Fill the json field list from the document.
  // .............................................................................

  hashElement->_subObjects = nullptr;
  int res = allocateSubObjectsHashIndexElement<T>(hashElement);

  if (res != TRI_ERROR_NO_ERROR) {
    // out of memory
    return res;
  }

  res = hashIndexHelper<T>(hashElement, document);

  // .............................................................................
  // It may happen that the document does not have the necessary attributes to
  // have particpated within the hash index. If the index is unique, we do not
  // report an error to the calling procedure, but return a warning instead. If
  // the index is not unique, we ignore this error.
  // .............................................................................

  if (res == TRI_ERROR_ARANGO_INDEX_DOCUMENT_ATTRIBUTE_MISSING && ! _unique) {
    res = TRI_ERROR_NO_ERROR;
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief inserts a data element into the hash array
///
/// Since we do not allow duplicates - we must compare using keys, rather than
/// documents.
////////////////////////////////////////////////////////////////////////////////

int HashIndex::insertUnique (TRI_hash_index_element_t const* element,
                             bool isRollback) {
  TRI_IF_FAILURE("InsertHashIndex") {
    return TRI_ERROR_DEBUG;
  }

  TRI_index_search_value_t key;
  int res = fillIndexSearchValueByHashIndexElement<TRI_hash_index_element_t>(&key, element);

  if (res != TRI_ERROR_NO_ERROR) {
    // out of memory
    return res;
  }

  res = TRI_InsertKeyHashArray(&_hashArray, &key, element, isRollback);

  if (key._values != nullptr) {
    TRI_Free(TRI_UNKNOWN_MEM_ZONE, key._values);
  }

  if (res == TRI_RESULT_KEY_EXISTS) {
    return TRI_set_errno(TRI_ERROR_ARANGO_UNIQUE_CONSTRAINT_VIOLATED);
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief removes an entry from the hash array part of the hash index
////////////////////////////////////////////////////////////////////////////////

int HashIndex::removeUnique (TRI_hash_index_element_t* element) {
  TRI_IF_FAILURE("RemoveHashIndex") {
    return TRI_ERROR_DEBUG;
  }

  int res = TRI_RemoveElementHashArray(&_hashArray, element);

  // this might happen when rolling back
  if (res == TRI_RESULT_ELEMENT_NOT_FOUND) {
    return TRI_ERROR_NO_ERROR;
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief locates a key within the hash array part
/// it is the callers responsibility to destroy the result
////////////////////////////////////////////////////////////////////////////////

TRI_hash_index_element_t* HashIndex::findUnique (TRI_index_search_value_t* key) {
  // .............................................................................
  // A find request means that a set of values for the "key" was sent. We need
  // to locate the hash array entry by key.
  // .............................................................................

  return TRI_FindByKeyHashArray(&_hashArray, key);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief inserts a data element into the hash array
///
/// Since we do allow duplicates - we must compare using documents, rather than
/// keys.
////////////////////////////////////////////////////////////////////////////////

int HashIndex::insertMulti (TRI_hash_index_element_multi_t* element,
                            bool isRollback) {
  TRI_IF_FAILURE("InsertHashIndex") {
    return TRI_ERROR_DEBUG;
  }

  TRI_index_search_value_t key;
  int res = fillIndexSearchValueByHashIndexElement<TRI_hash_index_element_multi_t>(&key, element);

  if (res != TRI_ERROR_NO_ERROR) {
    // out of memory
    return res;
  }

  res = TRI_InsertElementHashArrayMulti(&_hashArrayMulti, &key, element, isRollback);
  
  if (key._values != nullptr) {
    TRI_Free(TRI_UNKNOWN_MEM_ZONE, key._values);
  }

  if (res == TRI_RESULT_ELEMENT_EXISTS) {
    return TRI_ERROR_INTERNAL;
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief removes an entry from the associative array
////////////////////////////////////////////////////////////////////////////////

int HashIndex::removeMulti (TRI_hash_index_element_multi_t* element) {
  TRI_IF_FAILURE("RemoveHashIndex") {
    return TRI_ERROR_DEBUG;
  }

  TRI_index_search_value_t key;
  int res = fillIndexSearchValueByHashIndexElement<TRI_hash_index_element_multi_t>(&key, element);

  if (res != TRI_ERROR_NO_ERROR) {
    // out of memory
    return res;
  }

  res = TRI_RemoveElementHashArrayMulti(&_hashArrayMulti, &key, element);

  if (key._values != nullptr) {
    TRI_Free(TRI_UNKNOWN_MEM_ZONE, key._values);
  }

  if (res == TRI_RESULT_ELEMENT_NOT_FOUND) {
    return TRI_ERROR_INTERNAL;
  }

  return res;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
