////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC sub transaction class
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

#include "SubTransaction.h"
#include "Mvcc/TopLevelTransaction.h"
#include "Mvcc/Transaction.h"
#include "Mvcc/TransactionCollection.h"
#include "Mvcc/TransactionManager.h"
#include "Utils/Exception.h"
#include "VocBase/server.h"

using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                              class SubTransaction
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new sub transaction
////////////////////////////////////////////////////////////////////////////////

SubTransaction::SubTransaction (Transaction* parent,
                                std::map<std::string, bool> const& collections,
                                double ttl)
  : Transaction(parent->transactionManager(), TransactionId(TRI_NewTickServer(), parent->id().top()), parent->vocbase(), ttl),
    _topLevelTransaction(parent->topLevelTransaction()),
    _parentTransaction(parent) {
 
  _parentTransaction->subTransactionStarted(this);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the transaction
////////////////////////////////////////////////////////////////////////////////

SubTransaction::~SubTransaction () {
}

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief commit the transaction
////////////////////////////////////////////////////////////////////////////////
   
void SubTransaction::commit () {
  if (_status != Transaction::StatusType::ONGOING) {
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_TRANSACTION_INTERNAL, "cannot commit finished transaction");
  }
  
  // killed flag was set. must not commit!
  if (killed()) {
    return rollback();
  }

  if (_ongoingSubTransaction != nullptr) {
    try {
      _ongoingSubTransaction->rollback();
    }
    catch (...) {
    }
  }

  _status = StatusType::COMMITTED;
  _parentTransaction->subTransactionFinished(this);
  _transactionManager->deleteRunningTransaction(this);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief roll back the transaction
////////////////////////////////////////////////////////////////////////////////
   
void SubTransaction::rollback () {
  if (_status != Transaction::StatusType::ONGOING) {
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_TRANSACTION_INTERNAL, "cannot rollback finished transaction");
  }
  
  if (_ongoingSubTransaction != nullptr) {
    try {
      _ongoingSubTransaction->rollback();
    }
    catch (...) {
    }
  }

  _status = StatusType::ROLLED_BACK;
  _parentTransaction->subTransactionFinished(this);
  _transactionManager->deleteRunningTransaction(this);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns aggregated transaction statistics
////////////////////////////////////////////////////////////////////////////////

CollectionStats SubTransaction::aggregatedStats (TRI_voc_cid_t id) {
  // create empty stats first
  CollectionStats stats;

  auto current = static_cast<Transaction*>(this);
  while (current != nullptr) {
    auto it = current->_stats.find(id);
    if (it != current->_stats.end()) {
      // merge with already collected stats
      stats.merge((*it).second);
    }

    // until we're at the top
    current = current->parentTransaction();
  }

  return stats;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns a collection used in the transaction
/// this registers the collection in the transaction if not yet present
////////////////////////////////////////////////////////////////////////////////
        
TransactionCollection* SubTransaction::collection (std::string const& name) {
  return _topLevelTransaction->collection(name);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns a collection used in the transaction
/// this registers the collection in the transaction if not yet present
////////////////////////////////////////////////////////////////////////////////
        
TransactionCollection* SubTransaction::collection (TRI_voc_cid_t cid) {
  return _topLevelTransaction->collection(cid);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief prepare the statistics so updating them later is guaranteed to
/// succeed
////////////////////////////////////////////////////////////////////////////////

void SubTransaction::prepareStats (TransactionCollection const* collection) {
  auto cid = collection->id();
  auto it = _stats.find(cid);

  if (it == _stats.end()) {
    _stats.emplace(std::make_pair(cid, CollectionStats()));
  }
  
  _parentTransaction->prepareStats(collection);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief get a string representation of the transaction
////////////////////////////////////////////////////////////////////////////////

std::string SubTransaction::toString () const {
  std::string result("SubTransaction ");
  result += _id.toString();

  return result;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
