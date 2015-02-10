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
#include "Basics/fasthash.h"
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

class hashKey {
    size_t _nrFields;
  public:
    hashKey (size_t nrFields) : _nrFields(nrFields) {
    }
    ~hashKey () {
    }
    inline uint64_t operator() (HashIndex::Key const* key) {
      uint64_t hash = 0x0123456789abcdef;

      for (size_t j = 0; j < _nrFields; ++j) {
        // ignore the sid for hashing
        hash = fasthash64(key->at(j)._data.data, key->at(j)._data.length, hash);
      }

      return hash;
    }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief hash function looking at _key and _rev or only at _key
////////////////////////////////////////////////////////////////////////////////

class hashElement {
    size_t _nrFields;
  public:
    hashElement (size_t nrFields) : _nrFields(nrFields) {
    }
    ~hashElement () {
    }
    inline uint64_t operator() (HashIndex::Element const* elm, bool byKey) {
      uint64_t hash = 0x0123456789abcdef;

      TRI_shaped_json_t shaped;
      TRI_EXTRACT_SHAPED_JSON_MARKER(shaped, elm->_document->getDataPtr());

      char const* base = elm->_document->getShapedJsonPtr();
      for (size_t j = 0; j < _nrFields; ++j) {
        // ignore the sid for hashing
        char const* ptr = base + elm->_subObjects[j]._offset;

        // only hash the data block
        hash = fasthash64(ptr, elm->_subObjects[j]._length, hash);
      }
      if (byKey) {
        return hash;
      }

      // Now hash the key and the revision in as well:
      uint64_t dummy = elm->_document->getHash();
      hash = fasthash64(&dummy, sizeof(dummy), hash);
      dummy = elm->_document->_rid;
      hash = fasthash64(&dummy, sizeof(dummy), hash);

      return hash;
    }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief comparison function key/element
////////////////////////////////////////////////////////////////////////////////

class compareKeyElement {
    size_t _nrFields;
  public:
    compareKeyElement (size_t nrFields) : _nrFields(nrFields) {
    }
    ~compareKeyElement () {
    }
    inline bool operator() (HashIndex::Key const* key,
                            HashIndex::Element const* elm) {
      char const* base = elm->_document->getShapedJsonPtr();

      for (size_t j = 0; j < _nrFields; ++j) {
        TRI_shaped_json_t const* leftJson = &(key->at(j));
        TRI_shaped_sub_t const* rightSub = &(elm->_subObjects[j]);

        if (leftJson->_sid != rightSub->_sid) {
          return false;
        }

        auto length = leftJson->_data.length;

        if (length != rightSub->_length) {
          return false;
        }

        if (0 < length) {
          char const* ptr = base + rightSub->_offset;

          if (memcmp(leftJson->_data.data, ptr, length) != 0) {
            return false;
          }
        }
      }

      return true;
    }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief comparison function element/element
////////////////////////////////////////////////////////////////////////////////

class compareElementElement {
    size_t _nrFields;
  public:
    compareElementElement (size_t nrFields) : _nrFields(nrFields) {
    }
    ~compareElementElement () {
    }
    inline bool operator() (HashIndex::Element const* left,
                            HashIndex::Element const* right,
                            bool byKey) {
      char const* baseLeft = left->_document->getShapedJsonPtr();
      char const* baseRight = right->_document->getShapedJsonPtr();

      for (size_t j = 0; j < _nrFields; ++j) {
        TRI_shaped_sub_t const* leftSub = &(left->_subObjects[j]);
        TRI_shaped_sub_t const* rightSub = &(right->_subObjects[j]);

        if (leftSub->_sid != rightSub->_sid) {
          return false;
        }

        auto length = leftSub->_length;

        if (length != rightSub->_length) {
          return false;
        }

        if (0 < length) {
          char const* ptrLeft = baseLeft + leftSub->_offset;
          char const* ptrRight = baseRight + rightSub->_offset;

          if (memcmp(ptrLeft, ptrRight, length) != 0) {
            return false;
          }
        }
      }
      if (byKey) {
        return true;
      }
      if (left->_document->_rid != right->_document->_rid) {
        return false;
      }
      if (left->_document->getHash() != right->_document->getHash()) {
        return false;
      }
      char const* leftKey = TRI_EXTRACT_MARKER_KEY(left->_document);
      char const* rightKey = TRI_EXTRACT_MARKER_KEY(right->_document);
      if (strcmp(leftKey, rightKey) != 0) {
        return false;
      }
      return true;
    }
};

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
    _theHash = new Hash_t(hashKey(paths.size()), 
                          hashElement(paths.size()), 
                          compareKeyElement(paths.size()),
                          compareElementElement(paths.size()));
  }
  catch (...) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the hash index
////////////////////////////////////////////////////////////////////////////////

HashIndex::~HashIndex () {
  _theHash->iterate(
      [this] (HashIndex::Element* elm) { 
        deleteElement(elm); 
      } 
  );
  delete _theHash;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief insert a document into the index
////////////////////////////////////////////////////////////////////////////////
        
void HashIndex::insert (TransactionCollection* coll,
                        Transaction* trans,
                        TRI_doc_mptr_t* doc) {
  // We know that the revision doc is not yet stored in the index, because
  // it has been entered into the primary index without any trouble. Therefore
  // we can skip this check in the non-unique case.

  bool includeForSparse = true;

  if (! _unique) {
    WRITE_LOCKER(_lock);
    Element* hashElement = allocAndFillElement(coll, doc, includeForSparse);
    if (! _sparse || includeForSparse) {
      try {
        _theHash->insert(hashElement, false, false);
      }
      catch (...) {
        deleteElement(hashElement);
        throw;
      }
    }
  }
  else {  // _unique == true
    // See whether or not we find any revision that is in conflict with us.
    // Note that this could be a revision which we are not allowed to see!
    WRITE_LOCKER(_lock);
    Element* hashElement = allocAndFillElement(coll, doc, includeForSparse);
    if (! _sparse || includeForSparse) {
      try {
        std::unique_ptr<std::vector<Element*>> revisions 
            (_theHash->lookupWithElementByKey(hashElement));

        // We need to check whether or not there is any document/revision
        // that is in conflict with the new one, that is, its to() entry is
        // either empty or not committed before we started. Note that the from()
        // entry does not matter at all:
        for (auto p : *(revisions.get())) {
          TransactionId from = p->_document->from();
          TransactionId to = p->_document->to();
          Transaction::VisibilityType visFrom = trans->visibility(from);
          Transaction::VisibilityType visTo = trans->visibility(to);
          if (visFrom == Transaction::VisibilityType::VISIBLE) {
            if (visTo == Transaction::VisibilityType::VISIBLE) {
              continue;  // Ignore, has been made obsolete for us
            }
            else if (visTo == Transaction::VisibilityType::CONCURRENT) {
              THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_MVCC_WRITE_CONFLICT);
            }
            else {  // INVISIBLE, this includes to == 0
              THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_UNIQUE_CONSTRAINT_VIOLATED);
            }
          }
          else if (visFrom == Transaction::VisibilityType::CONCURRENT) {
            if (visTo != Transaction::VisibilityType::VISIBLE ) {
              THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_MVCC_WRITE_CONFLICT);
            }
            else {
              TRI_ASSERT(false);   // to cannot be VISIBLE unless from is
            }
          }
          else {
            TRI_ASSERT(visTo == Transaction::VisibilityType::INVISIBLE);
               // nothing else possible
            continue;
          }
        }
        _theHash->insert(hashElement, false, false);
      }
      catch (...) {
        deleteElement(hashElement);
        throw;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief remove a document from the index
////////////////////////////////////////////////////////////////////////////////

TRI_doc_mptr_t* HashIndex::remove (TransactionCollection*,
                                   Transaction*,
                                   std::string const&,
                                   TRI_doc_mptr_t*) {
  return nullptr;
}        

////////////////////////////////////////////////////////////////////////////////
/// @brief forget a document in the index
////////////////////////////////////////////////////////////////////////////////

void HashIndex::forget (TransactionCollection* coll,
                        Transaction*,
                        TRI_doc_mptr_t* doc) {
  bool dummy;
  std::unique_ptr<Element> hashElement(allocAndFillElement(coll, doc, dummy));
  WRITE_LOCKER(_lock);
  Element* old = _theHash->remove(hashElement.get());
  if (old == nullptr) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_KEYVALUE_KEY_NOT_FOUND);
  }
  deleteElement(old);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief post insert (does nothing)
////////////////////////////////////////////////////////////////////////////////
        
void HashIndex::preCommit (TransactionCollection*,
                           Transaction*) {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief lookup by key with limit, internal function
////////////////////////////////////////////////////////////////////////////////

std::vector<TRI_doc_mptr_t*>* HashIndex::lookupInternal(
                                  TransactionCollection* coll,
                                  Transaction* trans,
                                  Key const* key,
                                  TRI_doc_mptr_t* previousLast,
                                  size_t limit) {

  std::unique_ptr<std::vector<TRI_doc_mptr_t*>> theResult
        (new std::vector<TRI_doc_mptr_t*>);
  Element* previousLastElement = nullptr;
  Element* previousLastElementAlloc = nullptr;
  if (key != nullptr) {
    TRI_ASSERT(previousLast == nullptr);
  }
  else {
    TRI_ASSERT(previousLast != nullptr);
    bool dummy;
    previousLastElement = allocAndFillElement(coll, previousLast, dummy);
    previousLastElementAlloc = previousLastElement;
  }

  try {
    while (limit == 0 || theResult->size() < limit) {
      size_t subLimit = (limit == 0) ? 0 : limit - theResult->size();
      std::vector<Element*>* subResult;
      if (previousLastElement != nullptr) {
        subResult = _theHash->lookupByKey(key, subLimit);
      }
      else {
        subResult = _theHash->lookupByKeyContinue(previousLastElement,
                                                  subLimit);
        previousLastElement = subResult->back();
      }
      try {
        for (auto* d : *subResult) {
          TransactionId from = d->_document->from();
          TransactionId to = d->_document->to();
          if (trans->isVisibleForRead(from, to)) {
            theResult->push_back(d->_document);
          }
        }
        delete subResult;
      }
      catch (...) {
        delete subResult;
        throw;
      }
      if (limit == 0) {
        break;   // Otherwise we would loop forever
      }
    }
  }
  catch (...) {
    if (previousLastElementAlloc != nullptr) {
      deleteElement(previousLastElementAlloc);
    }
    throw;
  }

  return theResult.release();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief lookup by key with limit
////////////////////////////////////////////////////////////////////////////////

std::vector<TRI_doc_mptr_t*>* HashIndex::lookup (TransactionCollection* coll,
                                                 Transaction* trans,
                                                 Key const* key,
                                                 size_t limit = 0) {
  return lookupInternal(coll, trans, key, nullptr, limit);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief lookup by key with limit, continuation
////////////////////////////////////////////////////////////////////////////////

std::vector<TRI_doc_mptr_t*>* HashIndex::lookupContinue(
                                                  TransactionCollection* coll,
                                                  Transaction* trans,
                                                  TRI_doc_mptr_t* previousLast,
                                                  size_t limit = 0) {

  return lookupInternal(coll, trans, nullptr, previousLast, limit);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the memory used by the index
////////////////////////////////////////////////////////////////////////////////
  
size_t HashIndex::memory () {
  return _theHash->memoryUsage() + keySize() * _theHash->size();
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

HashIndex::Element* HashIndex::allocAndFillElement (TransactionCollection* coll,
                                                    TRI_doc_mptr_t* doc,
                                                    bool& includeForSparse) {
  HashIndex::Element* elm 
    = static_cast<HashIndex::Element*>(TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, 
                                                    keySize(), false));

  if (elm == nullptr) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
  }

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
  TRI_shaper_t* shaper = coll->shaper();

  size_t const n = _paths.size();

  includeForSparse = true;
  for (size_t i = 0; i < n; ++i) {
    TRI_shape_pid_t path = _paths[i];

    // determine if document has that particular shape
    TRI_shape_access_t const* acc 
          = TRI_FindAccessorVocShaper(shaper, shapedJson._sid, path);

    // field not part of the object
    if (acc == nullptr || acc->_resultSid == TRI_SHAPE_ILLEGAL) {
      shapedSub._sid    = TRI_LookupBasicSidShaper(TRI_SHAPE_NULL);
      shapedSub._length = 0;
      shapedSub._offset = 0;
      includeForSparse = false;
    }

    // extract the field
    else {
      TRI_shaped_json_t shapedObject;       // the sub-object

      if (! TRI_ExecuteShapeAccessor(acc, &shapedJson, &shapedObject)) {
        TRI_Free(TRI_UNKNOWN_MEM_ZONE, static_cast<void*>(elm));
        THROW_ARANGO_EXCEPTION(TRI_ERROR_INTERNAL);
      }

      if (shapedObject._sid == TRI_LookupBasicSidShaper(TRI_SHAPE_NULL)) {
        includeForSparse = false;
      }

      shapedSub._sid    = shapedObject._sid;
      shapedSub._length = shapedObject._data.length;
      shapedSub._offset = static_cast<uint32_t>(((char const*) shapedObject._data.data) - ptr);
    }

    // store the json shaped sub-object -- this is what will be hashed
    elm->_subObjects[i] = shapedSub;
  }

  return elm;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief deleteElement
////////////////////////////////////////////////////////////////////////////////

void HashIndex::deleteElement (Element* elm) {
  TRI_Free(TRI_UNKNOWN_MEM_ZONE, static_cast<void*>(elm));
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
