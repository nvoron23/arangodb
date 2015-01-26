////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC transaction collection object
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

#include "TransactionCollection.h"
#include "Mvcc/Index.h"
#include "Mvcc/MasterpointerManager.h"
#include "ShapedJson/json-shaper.h"
#include "Utils/Exception.h"
#include "VocBase/barrier.h"
#include "VocBase/document-collection.h"
#include "VocBase/key-generator.h"
#include "VocBase/vocbase.h"

using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                       class TransactionCollection
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create the collection, using a collection id
////////////////////////////////////////////////////////////////////////////////

TransactionCollection::TransactionCollection (TRI_vocbase_t* vocbase,
                                              TRI_voc_cid_t cid,
                                              Transaction* transaction)
  : _vocbase(vocbase),
    _collection(nullptr),
    _transaction(transaction),
    _barrier(nullptr) {

  _collection = TRI_UseCollectionByIdVocBase(_vocbase, cid);

  if (_collection == nullptr) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_COLLECTION_NOT_FOUND);
  }

  TRI_ASSERT(_collection->_collection != nullptr);
  orderBarrier();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create the collection, using a collection name
////////////////////////////////////////////////////////////////////////////////

TransactionCollection::TransactionCollection (TRI_vocbase_t* vocbase,
                                              std::string const& name,
                                              Transaction* transaction)
  : _vocbase(vocbase),
    _collection(nullptr),
    _transaction(transaction) {

  _collection = TRI_UseCollectionByNameVocBase(_vocbase, name.c_str());

  if (_collection == nullptr) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_COLLECTION_NOT_FOUND);
  }

  TRI_ASSERT(_collection->_collection != nullptr);
  orderBarrier();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the collection
////////////////////////////////////////////////////////////////////////////////

TransactionCollection::~TransactionCollection () {
  freeBarrier();

  if (_collection != nullptr) {
    TRI_ReleaseCollectionVocBase(_vocbase, _collection);
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief return the collection shaper
////////////////////////////////////////////////////////////////////////////////

TRI_shaper_t* TransactionCollection::shaper () const {
  return _collection->_collection->getShaper();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the collection's masterpointer manager
////////////////////////////////////////////////////////////////////////////////

MasterpointerManager* TransactionCollection::masterpointerManager () {
  return _collection->_collection->masterpointerManager();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief generate a key
////////////////////////////////////////////////////////////////////////////////

std::string TransactionCollection::generateKey (TRI_voc_tick_t tick) {
  return _collection->_collection->_keyGenerator->generate(tick);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief validate a key
/// this will throw if the key is invalid
////////////////////////////////////////////////////////////////////////////////

void TransactionCollection::validateKey (std::string const& key) const {
  // TODO: handle case isRestore
  int res = _collection->_collection->_keyGenerator->validate(key.c_str(), false);

  if (res != TRI_ERROR_NO_ERROR) {
    THROW_ARANGO_EXCEPTION(res);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief order a barrier for the collection
////////////////////////////////////////////////////////////////////////////////

TRI_barrier_t* TransactionCollection::orderBarrier () {
  if (_barrier == nullptr) {
    _barrier = TRI_CreateBarrierElement(&(documentCollection()->_barrierList));

    if (_barrier == nullptr) {
      THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_INTERNAL, "could not acquire barrier");
    }
             
    // tell everyone else this barrier is still in use,
    // at least until the transaction is over
    reinterpret_cast<TRI_barrier_blocker_t*>(_barrier)->_usedByTransaction = true;
  }

  return _barrier;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief free an unused barrier if one exists
////////////////////////////////////////////////////////////////////////////////

void TransactionCollection::freeBarrier () {
  if (_barrier == nullptr) {
    auto barrier = reinterpret_cast<TRI_barrier_blocker_t*>(_barrier);

    // conditional freeing of the barrier:
    // If some external entity is still using the barrier, it is kept!
    TRI_FreeBarrier(barrier, true /* fromTransaction */ );

    // we're done with this barrier
    _barrier = nullptr;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief get a string representation of the collection
////////////////////////////////////////////////////////////////////////////////

std::string TransactionCollection::toString () const {
  std::string result("TransactionCollection ");
  result.append(std::to_string(id()));
  result.append(", '");
  result.append(name());
  result.append("'");

  return result;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                          non-class friend methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief append the transaction to an output stream
////////////////////////////////////////////////////////////////////////////////

namespace triagens {
  namespace mvcc {

     std::ostream& operator<< (std::ostream& stream,
                               TransactionCollection const* collection) {
       stream << collection->toString();
       return stream;
     }

     std::ostream& operator<< (std::ostream& stream,
                               TransactionCollection const& collection) {
       stream << collection.toString();
       return stream;
     }
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
