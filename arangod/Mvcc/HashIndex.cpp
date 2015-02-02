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
#include "Basics/WriteLocker.h"
#include "HashIndex/hash-index.h"
#include "Utils/Exception.h"
#include "VocBase/document-collection.h"
#include "VocBase/voc-shaper.h"
#include "Mvcc/Transaction.h"
#include "Mvcc/TransactionCollection.h"

using namespace triagens::basics;
using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                                   class HashIndex
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                     hash and comparison functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief hash function only looking at the key
////////////////////////////////////////////////////////////////////////////////

static inline uint64_t hashKey (HashIndex::Key const* key) {
  // FIXME: implement
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief hash function looking at _key and _rev or only at _key
////////////////////////////////////////////////////////////////////////////////

static uint64_t hashElement (HashIndex::Element const* elm, bool byKey) {
  // FIXME: implement
  if (byKey) {
    return 0;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief comparison function key/element
////////////////////////////////////////////////////////////////////////////////

static bool compareKeyElement(HashIndex::Key const* key,
                              HashIndex::Element const* elm) {
  // FIXME: implement
  return false;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief comparison function element/element
////////////////////////////////////////////////////////////////////////////////

static bool compareElementElement(HashIndex::Element const* left,
                                  HashIndex::Element const* right,
                                  bool byKey) {
  // FIXME: implement
  if (byKey) {
    return false;
  }
  return false;
}

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
                      bool unique,
                      bool sparse) 
  : Index(id, collection, fields),
    _paths(paths),
    _theHash(nullptr),
    _unique(unique),
    _sparse(sparse) {
  
  try {
    _theHash = new Hash_t(hashKey, hashElement, compareKeyElement,
                          compareElementElement);
  }
  catch (...) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the hash index
////////////////////////////////////////////////////////////////////////////////

HashIndex::~HashIndex () {
  delete _theHash;
  // FIXME: do we have to free stuff in here?
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief insert a document into the index
////////////////////////////////////////////////////////////////////////////////
        
void HashIndex::insert (TransactionCollection* transColl,
                        TRI_doc_mptr_t* doc) {
  // FIXME: sparse not yet implemented

  // We know that the revision doc is not yet stored in the index, because
  // it has been entered into the primary index without any trouble. Therefore
  // we can skip this check in the non-unique case.

  if (! _unique) {
    WRITE_LOCKER(_lock);
    Element* hashElement = allocAndFillElement(doc);
    try {
      _theHash->insert(hashElement, false, false);
    }
    catch (...) {
      deleteElement(hashElement);
      throw;
    }
  }
  else {  // _unique == true
    // See whether or not we find any revision that is in conflict with us.
    // Note that this could be a revision which we are not allowed to see!
    WRITE_LOCKER(_lock);
    std::unique_ptr<Element> hashElement(allocAndFillElement(doc));
    std::unique_ptr<std::vector<Element*>> revisions 
        (_theHash->lookupWithElementByKey(hashElement));
    Transaction* trans = transColl->getTransaction();

    // We need to check whether or not there is any document/revision
    // that is in conflict with the new one, that is, its to() entry is
    // either empty or not committed before we started. Note that the from()
    // entry does not matter at all:
    for (auto p : *(revisions.get())) {
      TransactionId to = p->_document->to();
      if (to() == 0) {
        THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_UNIQUE_CONSTRAINT_VIOLATED);
      }
      Transaction::VisibilityType visibility = trans->visibility(to);
      if (visibility != Transaction::VisibilityType::VISIBLE) {
        visibility = trans->visibility(p->_document->from());
        if (visibility == Transaction::VisibilityType::VISIBLE) {
          THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_UNIQUE_CONSTRAINT_VIOLATED);
        }
        else {
          THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_MVCC_WRITE_CONFLICT);
        }
      }
    }
    _theHash->insert(hashElement.get(), false, false);
    hashElement.reset(nullptr);  // safely stored in hash table
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief remove a document from the index
////////////////////////////////////////////////////////////////////////////////

TRI_doc_mptr_t* HashIndex::remove (TransactionCollection*,
                                   std::string const&,
                                   TRI_doc_mptr_t const*) {
  return nullptr;
}        

////////////////////////////////////////////////////////////////////////////////
/// @brief forget a document in the index
////////////////////////////////////////////////////////////////////////////////

void HashIndex::forget (TransactionCollection*,
                        TRI_doc_mptr_t const* doc) {
  std::unique_ptr<Element> hashElement(allocAndFillElement(doc));
  WRITE_LOCK(_lock);
  Element* old = _theHash->remove(hashElement);
  if (old == nullptr) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_KEYVALUE_KEY_NOT_FOUND);
  }
  deleteElement(old);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief post insert (does nothing)
////////////////////////////////////////////////////////////////////////////////
        
void HashIndex::preCommit (TransactionCollection*) {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief provides a hint for the expected index size
////////////////////////////////////////////////////////////////////////////////

void HashIndex::sizeHint (size_t size) {
  _theHash->resize(3*size + 1);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the memory used by the index
////////////////////////////////////////////////////////////////////////////////
  
size_t HashIndex::memory () {
  return theHash->memoryUsage() + keySize() * _theHash->size();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return a JSON representation of the index
////////////////////////////////////////////////////////////////////////////////

Json HashIndex::toJson (TRI_memory_zone_t* zone) const {
  Json json = Index::toJson(zone);
  Json fields(zone, Json::Array, _fields.size());
  for (auto& field : _fields) {
    fields.add(Json(zone, field));
  }
  json("fields", fields)
      ("unique", Json(zone, _unique))
      ("sparse", Json(zone, _sparse));
  return json;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the memory needed for an index key entry
////////////////////////////////////////////////////////////////////////////////

size_t HashIndex::keySize () const {
  return sizeof(TRI_doc_mptr_t*) + _paths.size() * sizeof(TRI_shaped_sub_t);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief allocAndFillElement
////////////////////////////////////////////////////////////////////////////////

Element* HashIndex::allocAndFillElement (TRI_doc_mptr_t* doc) {
  auto elm = static_cast<Element*>(TRI_Alloc(TRI_UNKNOWN_MEM_ZONE, keySize()));
  elm->_document = doc;
  // ...........................................................................
  // Assign the document to the Element structure - so that it can later
  // be retreived.
  // ...........................................................................

  TRI_shaped_json_t shapedJson;
  TRI_EXTRACT_SHAPED_JSON_MARKER(shapedJson, doc->getDataPtr());

  char const* ptr = doc->getShapedJsonPtr();

  // ...........................................................................
  // Extract the attribute values
  // ...........................................................................

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


}

////////////////////////////////////////////////////////////////////////////////
/// @brief deleteElement
////////////////////////////////////////////////////////////////////////////////

void HashIndex::deleteElement (Element* elm) {
  TRI_Free(TRI_UNKNOWN_MEM_ZONE, static_cast<void*>(elm));
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
