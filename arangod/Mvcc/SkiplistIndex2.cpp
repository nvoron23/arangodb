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
/// @author Max Neunhoeffer
/// @author Copyright 2015, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "SkiplistIndex2.h"
#include "Basics/ReadLocker.h"
#include "Basics/WriteLocker.h"
#include "Mvcc/Transaction.h"
#include "Mvcc/TransactionCollection.h"
#include "Utils/Exception.h"
#include "VocBase/document-collection.h"
#include "VocBase/voc-shaper.h"

using namespace triagens::basics;
using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                               class SkiplistIndex
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                   private helpers
// -----------------------------------------------------------------------------

// .............................................................................
// recall for all of the following comparison functions:
//
// left < right  return -1
// left > right  return  1
// left == right return  0
//
// furthermore:
//
// the following order is currently defined for placing an order on documents
// undef < null < boolean < number < strings < lists < hash arrays
// note: undefined will be treated as NULL pointer not NULL JSON OBJECT
// within each type class we have the following order
// boolean: false < true
// number: natural order
// strings: lexicographical
// lists: lexicographically and within each slot according to these rules.
// ...........................................................................

////////////////////////////////////////////////////////////////////////////////
/// @brief compares a key with an element, version with proper types
////////////////////////////////////////////////////////////////////////////////

static int CompareKeyElement (TRI_shaped_json_t const* left,
                              SkiplistIndex2::Element* right,
                              size_t rightPosition,
                              TRI_shaper_t* shaper) {
  int result;

  TRI_ASSERT(nullptr != left);
  TRI_ASSERT(nullptr != right);
  result = TRI_CompareShapeTypes(nullptr,
                                 nullptr,
                                 left,
                                 shaper,
                                 right->_document->getShapedJsonPtr(),
                                 &right->_subObjects[rightPosition],
                                 nullptr,
                                 shaper);

  // ...........................................................................
  // In the above function CompareShapeTypes we use strcmp which may
  // return an integer greater than 1 or less than -1. From this
  // function we only need to know whether we have equality (0), less
  // than (-1) or greater than (1)
  // ...........................................................................

  if (result < 0) {
    result = -1;
  }
  else if (result > 0) {
    result = 1;
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief compares elements, version with proper types
////////////////////////////////////////////////////////////////////////////////

static int CompareElementElement (SkiplistIndex2::Element* left,
                                  size_t leftPosition,
                                  SkiplistIndex2::Element* right,
                                  size_t rightPosition,
                                  TRI_shaper_t* shaper) {
  TRI_ASSERT(nullptr != left);
  TRI_ASSERT(nullptr != right);

  int result = TRI_CompareShapeTypes(left->_document->getShapedJsonPtr(),
                                     &left->_subObjects[leftPosition],
                                     nullptr,
                                     shaper,
                                     right->_document->getShapedJsonPtr(),
                                     &right->_subObjects[rightPosition],
                                     nullptr,
                                     shaper);

  // ...........................................................................
  // In the above function CompareShapeTypes we use strcmp which may
  // return an integer greater than 1 or less than -1. From this
  // function we only need to know whether we have equality (0), less
  // than (-1) or greater than (1)
  // ...........................................................................

  if (result < 0) {
    return -1;
  }
  else if (result > 0) {
    return 1;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief compares two elements in a skip list, this is the generic callback
////////////////////////////////////////////////////////////////////////////////

class CmpElmElm {
    TRI_shaper_t* _shaper;
    size_t _numFields;
  public:
    CmpElmElm (TRI_shaper_t* shaper, size_t numFields) 
      : _shaper(shaper), _numFields(numFields) {
    }
    ~CmpElmElm () {
    }
    int operator() (SkiplistIndex2::Element* leftElement, 
                    SkiplistIndex2::Element* rightElement, 
                    SkiplistIndex2::SkipList_t::CmpType cmptype) {
      TRI_ASSERT(nullptr != leftElement);
      TRI_ASSERT(nullptr != rightElement);

      // .......................................................................
      // The document could be the same -- so no further comparison is required.
      // .......................................................................

      if (leftElement == rightElement ||
          leftElement->_document == rightElement->_document) {
        return 0;
      }

      int compareResult;
      for (size_t j = 0;  j < _numFields;  j++) {
        compareResult = CompareElementElement(leftElement,
                                              j,
                                              rightElement,
                                              j,
                                              _shaper);

        if (compareResult != 0) {
          return compareResult;
        }
      }

      // .......................................................................
      // This is where the difference between the preorder and the proper total
      // order comes into play. Here if the 'keys' are the same,
      // but the doc ptr is different (which it is since we are here), then
      // we return 0 if we use the preorder and look at the _key attribute
      // otherwise.
      // .......................................................................

      if (SkiplistIndex2::SkipList_t::CMP_PREORDER == cmptype) {
        return 0;
      }

      // We break this tie in the key comparison by first looking at the key...
      compareResult = strcmp(TRI_EXTRACT_MARKER_KEY(leftElement->_document),
                             TRI_EXTRACT_MARKER_KEY(rightElement->_document));

      if (compareResult < 0) {
        return -1;
      }
      else if (compareResult > 0) {
        return 1;
      }
      // ... and then at the revision:
      if (leftElement->_document->_rid < rightElement->_document->_rid) {
        return -1;
      }
      else if (leftElement->_document->_rid > rightElement->_document->_rid) {
        return 1;
      }
      return 0;
    }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief compares a key with an element in a skip list, generic callback
////////////////////////////////////////////////////////////////////////////////

class CmpKeyElm {
    TRI_shaper_t* _shaper;
    size_t _numFields;
  public:
    CmpKeyElm (TRI_shaper_t* shaper, size_t numFields) 
      : _shaper(shaper), _numFields(numFields) {
    }
    ~CmpKeyElm () {
    }
    int operator() (SkiplistIndex2::Key*     leftKey, 
                    SkiplistIndex2::Element* rightElement) {
      TRI_ASSERT(nullptr != leftKey);
      TRI_ASSERT(nullptr != rightElement);

      // Note that the key might contain fewer fields than there are indexed
      // attributes, therefore we only run the following loop to
      // leftKey->_numFields.
      for (size_t j = 0;  j < leftKey->size();  j++) {
        int compareResult = CompareKeyElement(&leftKey->at(j), 
                                              rightElement, j, _shaper);

        if (compareResult != 0) {
          return compareResult;
        }
      }

      return 0;
    }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief frees an element in the skiplist
////////////////////////////////////////////////////////////////////////////////

class FreeElm {
    SkiplistIndex2* _skipListIndex;
  public:
    FreeElm (SkiplistIndex2* skipListIndex)
      : _skipListIndex(skipListIndex) {
    }
    ~FreeElm () {
    }
    void operator() (SkiplistIndex2::Element* elm) {
      _skipListIndex->deleteElement(elm);
    }
};

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
                                bool sparse)
  : Index(id, collection, fields),
    _paths(paths),
    _theSkipList(nullptr),
    _unique(unique),
    _sparse(sparse) {
  
  try {
    _theSkipList = new SkipList_t(CmpElmElm(collection->getShaper(),
                                            paths.size()),
                                  CmpKeyElm(collection->getShaper(),
                                            paths.size()),
                                  FreeElm(this),
                                  unique);
  }
  catch (...) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the skiplist index
////////////////////////////////////////////////////////////////////////////////

SkiplistIndex2::~SkiplistIndex2 () {
  if (_theSkipList != nullptr) {
    delete _theSkipList;
    _theSkipList = nullptr;
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------
        
std::vector<TRI_doc_mptr_t*>* SkiplistIndex2::lookup (TransactionCollection* coll,
                                                      Transaction* trans,
                                                      TRI_index_operator_t* op,
                                                      size_t skip,
                                                      size_t limit,
                                                      bool reverse) {
  
  std::unique_ptr<std::vector<TRI_doc_mptr_t*>> theResult(new std::vector<TRI_doc_mptr_t*>);
  // TODO: this has to be implemented still!

  return theResult.release();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief insert document into index
////////////////////////////////////////////////////////////////////////////////
        
void SkiplistIndex2::insert (TransactionCollection* coll,
                             Transaction* trans,
                             TRI_doc_mptr_t* doc) {
  bool includeForSparse = true;

  if (! _unique) {
    WRITE_LOCKER(_lock);
    Element* listElement = allocateAndFillElement(coll, doc, includeForSparse);
    TRI_ASSERT(listElement != nullptr);

    if (! _sparse || includeForSparse) {
      try {
        _theSkipList->insert(listElement);
      }
      catch (...) {
        deleteElement(listElement);
        throw;
      }
    }
    else {
      deleteElement(listElement);
    }
  }
  else {  // _unique == true
    // See whether or not we find any revision that is in conflict with us.
    // Note that this could be a revision which we are not allowed to see!
    WRITE_LOCKER(_lock);
    Element* listElement = allocateAndFillElement(coll, doc, includeForSparse);
    if (! _sparse || includeForSparse) {
      try {
        // Find the last element in the list whose key is less than ours:
        SkipList_t::Node* node = _theSkipList->leftLookup(listElement);
        while (true) {   // will be left by break
          node = node->nextNode();
          if (node == nullptr) {
            break;
          }
          Element* p = node->document();
          CmpElmElm comparer(coll->shaper(), nrIndexedFields());
          if (comparer(listElement, p, SkipList_t::CMP_PREORDER) != 0) {
            break;
          }
          TransactionId from = p->_document->from();
          TransactionId to = p->_document->to();
          Transaction::VisibilityType visFrom = trans->visibility(from);
          Transaction::VisibilityType visTo = trans->visibility(to);
          if (visFrom == Transaction::VisibilityType::VISIBLE) {
            if (visTo == Transaction::VisibilityType::VISIBLE) {
              continue;  // Ignore, has been made obsolete for us
            }
            else if (visTo == Transaction::VisibilityType::CONCURRENT) {
              deleteElement(listElement);
              THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_MVCC_WRITE_CONFLICT);
            }
            else {  // INVISIBLE, this includes to == 0
              deleteElement(listElement);
              THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_UNIQUE_CONSTRAINT_VIOLATED);
            }
          }
          else if (visFrom == Transaction::VisibilityType::CONCURRENT) {
            if (visTo != Transaction::VisibilityType::VISIBLE ) {
              deleteElement(listElement);
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
        _theSkipList->insert(listElement);
      }
      catch (...) {
        deleteElement(listElement);
        throw;
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief remove document from index
////////////////////////////////////////////////////////////////////////////////
        
TRI_doc_mptr_t* SkiplistIndex2::remove (TransactionCollection*,
                                        Transaction*,
                                        std::string const&,
                                        TRI_doc_mptr_t* doc) {
  return nullptr;   // this is a no-operation
}

////////////////////////////////////////////////////////////////////////////////
/// @brief forget document in the index
////////////////////////////////////////////////////////////////////////////////
        
void SkiplistIndex2::forget (TransactionCollection* coll,
                             Transaction* trans,
                             TRI_doc_mptr_t* doc) {
  bool dummy;
  Element* listElement = allocateAndFillElement(coll, doc, dummy);

  try {
    WRITE_LOCKER(_lock);
    int res = _theSkipList->remove(listElement);
    
    if (res != TRI_ERROR_NO_ERROR) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_KEYVALUE_KEY_NOT_FOUND);
    }
  }
  catch (...) {
    deleteElement(listElement);
    throw;
  }
  deleteElement(listElement);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief post-insert (does nothing)
////////////////////////////////////////////////////////////////////////////////

void SkiplistIndex2::preCommit (TransactionCollection*, Transaction*) {
  // this is intentionally a no-operation
}

////////////////////////////////////////////////////////////////////////////////
/// @brief determine the amount of memory used by the index
////////////////////////////////////////////////////////////////////////////////

size_t SkiplistIndex2::memory () {
  READ_LOCKER(_lock);
  return _theSkipList->memoryUsage() + elementSize() * _theSkipList->getNrUsed();
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
      ("sparse", Json(zone, _sparse));
  return json;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief cleanup
////////////////////////////////////////////////////////////////////////////////
        
// a garbage collection function for the index
void SkiplistIndex2::cleanup () {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief sizeHint - ignored
////////////////////////////////////////////////////////////////////////////////
        
// give index a hint about the expected size
void SkiplistIndex2::sizeHint (size_t) {
}

  
// -----------------------------------------------------------------------------
// --SECTION--                                                 private functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the memory needed for an index key entry
////////////////////////////////////////////////////////////////////////////////

size_t SkiplistIndex2::elementSize () const {
  return sizeof(TRI_doc_mptr_t*) + _paths.size() * sizeof(TRI_shaped_sub_t);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief allocateAndFillElement
////////////////////////////////////////////////////////////////////////////////

SkiplistIndex2::Element* SkiplistIndex2::allocateAndFillElement (
                                 TransactionCollection* coll,
                                 TRI_doc_mptr_t* doc,
                                 bool& includeForSparse) {
  auto elm 
    = static_cast<SkiplistIndex2::Element*>(TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, 
                                                         elementSize(),
                                                         false));

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
      elm->_subObjects[i]._sid = BasicShapes::TRI_SHAPE_SID_NULL;
      includeForSparse = false;
    }

    // extract the field
    else {
      TRI_shaped_json_t shapedObject;       // the sub-object

      if (! TRI_ExecuteShapeAccessor(acc, &shapedJson, &shapedObject)) {
        TRI_Free(TRI_UNKNOWN_MEM_ZONE, static_cast<void*>(elm));
        THROW_ARANGO_EXCEPTION(TRI_ERROR_INTERNAL);
      }

      if (shapedObject._sid == BasicShapes::TRI_SHAPE_SID_NULL) {
        includeForSparse = false;
      }

      TRI_FillShapedSub(&elm->_subObjects[i], &shapedObject, ptr);
    }
  }

  return elm;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief deleteElement
////////////////////////////////////////////////////////////////////////////////

void SkiplistIndex2::deleteElement (Element* elm) {
  TRI_Free(TRI_UNKNOWN_MEM_ZONE, static_cast<void*>(elm));
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
