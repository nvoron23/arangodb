////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC top-level transaction class
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

#include "TopLevelTransaction.h"
#include "Basics/logging.h"
#include "Mvcc/TransactionCollection.h"
#include "Mvcc/TransactionManager.h"
#include "Utils/Exception.h"
#include "VocBase/vocbase.h"
#include "Wal/LogfileManager.h"

using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                         class TopLevelTransaction
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new top-level transaction
////////////////////////////////////////////////////////////////////////////////

TopLevelTransaction::TopLevelTransaction (TransactionManager* transactionManager,
                                          TransactionId const& id,
                                          TRI_vocbase_t* vocbase)
  : Transaction(transactionManager, id, vocbase),
    _runningTransactions(nullptr) {

  LOG_TRACE("creating %s", toString().c_str());
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the transaction
////////////////////////////////////////////////////////////////////////////////

TopLevelTransaction::~TopLevelTransaction () {
  LOG_TRACE("destroying %s", toString().c_str());

  if (_status == Transaction::StatusType::ONGOING) {
    try {
      rollback();
    }
    catch (...) {
      // destructors better not throw
    }
  }
  
  if (_runningTransactions != nullptr) {
    delete _runningTransactions;
  }

  // go through all the collections that have been registered and close them properly
  for (auto it : _collectionIds) {
    delete it.second;
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief commit the transaction
////////////////////////////////////////////////////////////////////////////////

int TopLevelTransaction::commit () {
  // TODO: implement locking here in case multiple threads access the same transaction
  LOG_TRACE("committing transaction %s", toString().c_str());

  if (_status != Transaction::StatusType::ONGOING) {
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_TRANSACTION_INTERNAL, "cannot commit finished transaction");
  }

  // killed flag was set. must not commit!
  if (killed()) {
    return rollback();
  }

  // loop over all statistics to check if there were any data modifications during 
  // the transaction
  bool transactionContainsModification = false;
  bool waitForSync = false;

  for (auto const& it : _stats) {
    transactionContainsModification |= (it.second.numInserted >0 || it.second.numRemoved > 0);

    // check if we must sync
    waitForSync |= it.second.waitForSync;
  }

  if (transactionContainsModification) {
    try {
      // write a commit marker
      triagens::wal::MvccCommitTransactionMarker commitMarker(_vocbase->_id, _id);

      auto logfileManager = triagens::wal::LogfileManager::instance();
      int res = logfileManager->allocateAndWrite(commitMarker, waitForSync).errorCode;
      
      if (res != TRI_ERROR_NO_ERROR) {
        rollback();
        THROW_ARANGO_EXCEPTION(res); // will be caught below
      }
    }
    catch (...) {
      // always roll back
      rollback();

      THROW_ARANGO_EXCEPTION(TRI_ERROR_INTERNAL);
    }

    // finally check how many documents were changed in each collection
    // and update the collection stats
    for (auto const& it : _stats) {
      int64_t difference = static_cast<int64_t>(it.second.numInserted - it.second.numRemoved);

      if (difference != 0) {
        auto it2 = _collectionIds.find(it.first);

        if (it2 != _collectionIds.end()) {
          (*it2).second->updateDocumentCounter(difference);
        }
      }
    }
  }
    
  _status = StatusType::COMMITTED;
  _transactionManager->removeRunningTransaction(this, transactionContainsModification);

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief roll back the transaction
////////////////////////////////////////////////////////////////////////////////

int TopLevelTransaction::rollback () {
  // TODO: implement locking here in case multiple threads access the same transaction
  LOG_TRACE("rolling back transaction %s", toString().c_str());

  if (_status != Transaction::StatusType::ONGOING) {
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_TRANSACTION_INTERNAL, "cannot rollback finished transaction");
  }

  _status = StatusType::ROLLED_BACK;

  // mark all subtransactions as failed, too
  for (auto& it : _subTransactions) {
    it.second = StatusType::ROLLED_BACK;
  }
  
  bool transactionContainsModification = false;

  for (auto const& it : _stats) {
    if (it.second.numInserted >0 || it.second.numRemoved > 0) {
      transactionContainsModification = true;
      break;
    }
  }
  
  if (transactionContainsModification) {
    try {
      // write an abort marker
      triagens::wal::MvccAbortTransactionMarker abortMarker(_vocbase->_id, _id);
    
      auto logfileManager = triagens::wal::LogfileManager::instance();
      logfileManager->allocateAndWrite(abortMarker, false).errorCode;
    }
    catch (...) {
      // what to do if writing an abort marker failed?
    }
  }

  _transactionManager->removeRunningTransaction(this, transactionContainsModification);
  
  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns aggregated transaction statistics
////////////////////////////////////////////////////////////////////////////////

CollectionStats TopLevelTransaction::aggregatedStats (TRI_voc_cid_t id) {
  // create empty stats first
  CollectionStats stats;

  auto it = _stats.find(id);
  if (it != _stats.end()) {
    stats.merge((*it).second);
  }

  return stats;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns a collection used in the transaction
/// this registers the collection in the transaction if not yet present
/// TODO: make this cluster-aware!
////////////////////////////////////////////////////////////////////////////////
        
TransactionCollection* TopLevelTransaction::collection (std::string const& name) {
  if (name[0] >= '0' && name[0] <= '9') {
    // name is a numeric id
    try {
      // convert string to number, and call the function for numeric collection id
      TRI_voc_cid_t cid = static_cast<TRI_voc_cid_t>(std::stoull(name));
      return collection(cid);
    }
    catch (...) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_COLLECTION_NOT_FOUND);
    }
  }
 
  // look up the collection name in our cache
  auto it = _collectionNames.find(name);

  if (it != _collectionNames.end()) {
    // collection name is in cache already
    return (*it).second;
  }

  // not found. now create it. note: this may throw 
  return registerCollection(new TransactionCollection(_vocbase, name, this));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns a collection used in the transaction
/// this registers the collection in the transaction if not yet present
////////////////////////////////////////////////////////////////////////////////
        
TransactionCollection* TopLevelTransaction::collection (TRI_voc_cid_t id) {
  // look up the collection id in our cache
  auto it = _collectionIds.find(id);

  if (it != _collectionIds.end()) {
    // collection id is in cache already
    return (*it).second;
  }

  // not found. now create it. note: this may throw 
  return registerCollection(new TransactionCollection(_vocbase, id, this));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief get a string representation of the transaction
////////////////////////////////////////////////////////////////////////////////

std::string TopLevelTransaction::toString () const {
  std::string result("TopLevelTransaction ");
  result += _id.toString();

  return result;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief sets the start state (e.g. list of running transactions
////////////////////////////////////////////////////////////////////////////////

void TopLevelTransaction::setStartState (std::unordered_map<TransactionId::InternalType, Transaction*> const& transactions) {
  TRI_ASSERT(_runningTransactions == nullptr);

  if (! transactions.empty()) {
    _runningTransactions = new std::unordered_set<TransactionId::InternalType>();
    _runningTransactions->reserve(transactions.size());

    for (auto it : transactions) {
      _runningTransactions->emplace(it.first);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief registers a collection inside the transaction
/// if this throws, then the collection object will be deleted by the method!
/// if this succeeds, the _collectionIds map has the ownership for the 
/// collection object
////////////////////////////////////////////////////////////////////////////////

TransactionCollection* TopLevelTransaction::registerCollection (TransactionCollection* collection) {
  TRI_ASSERT(collection != nullptr);

  try {
    _collectionNames.emplace(std::make_pair(collection->name(), collection));
    try {
      _collectionIds.emplace(std::make_pair(collection->id(), collection));
    }
    catch (...) {
      // do proper cleanup
      _collectionNames.erase(collection->name());
      throw;
    }
  }
  catch (...) {
    // prevent memleak
    delete collection;
    throw;
  }

  return collection;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
