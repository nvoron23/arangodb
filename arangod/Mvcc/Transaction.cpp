////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC transaction class
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

#include "Transaction.h"
#include "Basics/logging.h"
#include "Mvcc/TopLevelTransaction.h"
#include "Mvcc/TransactionCollection.h"
#include "Mvcc/TransactionManager.h"
#include "Utils/Exception.h"
#include "VocBase/vocbase.h"

using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                                 class Transaction
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief reserve room for committed transactions
////////////////////////////////////////////////////////////////////////////////

size_t const Transaction::ReserveRoomForCommitted = 8;

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new transaction, with id provided by the transaction
/// manager
////////////////////////////////////////////////////////////////////////////////

Transaction::Transaction (TransactionManager* transactionManager,
                          TransactionId const& id,
                          TRI_vocbase_t* vocbase,
                          double ttl)
  : _transactionManager(transactionManager),
    _id(id),
    _vocbase(vocbase),
    _startTime(TRI_microtime()),
    _status(Transaction::StatusType::ONGOING),
    _ongoingSubTransaction(nullptr),
    _committedSubTransactions(),
    _stats(),
    _expireTime(0.0),
    _killed(false) {

  // avoid expensive time call if no ttl was specified
  if (ttl > 0.0) {
    _expireTime = TRI_microtime() + ttl;
  }

  TRI_ASSERT(_transactionManager != nullptr);
  TRI_ASSERT(_vocbase != nullptr);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the transaction
////////////////////////////////////////////////////////////////////////////////

Transaction::~Transaction () {
  TRI_ASSERT(_ongoingSubTransaction == nullptr);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not the transaction contains successful data-modifying 
/// operations (failed operations are not included)
////////////////////////////////////////////////////////////////////////////////

bool Transaction::hasModifications () const {
  for (auto const& it : _stats) {
    if (it.second.hasModifications()) {
      return true;
    }
  }
  return false;
} 

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not the transaction must sync on commit
////////////////////////////////////////////////////////////////////////////////

bool Transaction::hasWaitForSync () const {
  for (auto const& it : _stats) {
    // check if we must sync
    if (it.second.waitForSync) {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief prepare the statistics so updating them later is guaranteed to
/// succeed
////////////////////////////////////////////////////////////////////////////////

void Transaction::prepareStats (TransactionCollection const* collection) {
  auto cid = collection->id();
  auto it = _stats.find(cid);

  if (it == _stats.end()) {
    _stats.emplace(std::make_pair(cid, CollectionStats()));
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief update the number of inserted documents
/// this is guaranteed to not fail if prepareStats() was called before
////////////////////////////////////////////////////////////////////////////////

void Transaction::incNumInserted (TransactionCollection const* collection,
                                  bool waitForSync) {
  auto cid = collection->id();
  auto it = _stats.find(cid);
  
  TRI_ASSERT(it != _stats.end());
  (*it).second.numInserted++;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief update the number of removed documents
/// this is guaranteed to not fail if prepareStats() was called before
////////////////////////////////////////////////////////////////////////////////

void Transaction::incNumRemoved (TransactionCollection const* collection,
                                 bool waitForSync) {
  auto cid = collection->id();
  auto it = _stats.find(cid);
  
  TRI_ASSERT(it != _stats.end());

  (*it).second.numRemoved++;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief update the last revision id of the collection
/// this is guaranteed to not fail if prepareStats() was called before
////////////////////////////////////////////////////////////////////////////////

void Transaction::updateRevisionId (TransactionCollection const* collection,
                                    TRI_voc_rid_t revisionId) {
  auto cid = collection->id();
  auto it = _stats.find(cid);
  
  TRI_ASSERT(it != _stats.end());

  (*it).second.updateRevision(revisionId);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                 protected methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief this is called when a sub-transaction is started
/// this reserves enough space in the committedSubTransactions element so
/// it will be able to capture the transaction id on commit 
////////////////////////////////////////////////////////////////////////////////

void Transaction::subTransactionStarted (Transaction* transaction) {
  size_t const n = _committedSubTransactions.size();
  
  // reserve enough space in the container so later commits won't fail with
  // out of memory
  _committedSubTransactions.reserve(n + ReserveRoomForCommitted);

  // this will recursively inform all parent transactions
  if (! isTopLevel()) {
    parentTransaction()->subTransactionStarted(this);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief this is called when a sub-transaction is finished
////////////////////////////////////////////////////////////////////////////////

void Transaction::subTransactionFinished (Transaction* transaction) {
  auto const id = transaction->id();
  auto const status = transaction->status();
 
  _ongoingSubTransaction = nullptr;

  if (status == StatusType::COMMITTED) { 
    bool hasAnyModifications = false;

    // sub-transaction has committed, now copy its stats into our stats
    for (auto const& it : transaction->_stats) {
      if (! it.second.hasModifications()) {
        continue;
      }

      hasAnyModifications = true;

      // _stats entry must have been created earlier using prepareStats()
      auto it2 = _stats.find(it.first);
      TRI_ASSERT(it2 != _stats.end());
      // merge our own stats with the sub-transaction's stats
      (*it2).second.merge(it.second);
    }
    
    // track the id of the transaction, but only if the transaction has actually
    // modified data
    if (hasAnyModifications) {
      // this shouldn't fail as we explicitly increated the capacity before 
      _committedSubTransactions.emplace(id.own());
    }

    // copy all committed sub-transactions of other into ourselves
    auto const& cst = transaction->_committedSubTransactions;
    // this shouldn't fail as we explicitly increated the capacity before 
    _committedSubTransactions.insert(cst.begin(), cst.end());
  }
  else {
    // remove the transaction from the list of already committed 
    // sub-transactions, as it might be in there!
    _transactionManager->addFailedTransactions(transaction->_committedSubTransactions);
    transaction->_committedSubTransactions.clear();
  }
  
  // now delete the stats of the sub-transaction as they are not needed anymore
  transaction->_stats.clear();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief visibility, this implements the MVCC logic of what this transaction
/// can see, returns the visibility of the other transaction for this one.
/// The result can be INVISIBLE, CONCURRENT or VISIBLE. We guarantee
/// INVISIBLE < CONCURRENT < VISIBLE, such that one can do things like
/// "visibility(other) < VISIBLE" legally.
////////////////////////////////////////////////////////////////////////////////
    
Transaction::VisibilityType Transaction::visibility (TransactionId const& other) const {
  if (other.own() == 0) {
    return VisibilityType::INVISIBLE;
  }
  if (other.own() == 1) {
    return VisibilityType::VISIBLE;
  }

  if (_id.top() == other.top()) {   // same top level transaction?
    if (other.own() > _id.own()) {
      if (_transactionManager->statusTransaction(other) == StatusType::ROLLED_BACK) {
        return VisibilityType::INVISIBLE;
      }
      if (statusSubTransaction(other) == StatusType::COMMITTED) {
        return VisibilityType::VISIBLE;
      }
      return VisibilityType::CONCURRENT;
    }

    if (other.own() < _id.own()) {
      if (_transactionManager->statusTransaction(other) == StatusType::ROLLED_BACK) {
        return VisibilityType::INVISIBLE;
      }
      return VisibilityType::VISIBLE;
    }

    // same transaction!
    return VisibilityType::VISIBLE;
  }

  if (other.top() > _id.top() || 
      topLevelTransaction()->wasOngoingAtStart(other.top())) {
    if (_transactionManager->statusTransaction(other) == StatusType::ROLLED_BACK) {
      return VisibilityType::INVISIBLE;
    }
    
    return VisibilityType::CONCURRENT;
  }

  TRI_ASSERT(other.top() < _id.top());
    
  if (_transactionManager->statusTransaction(other) == StatusType::ROLLED_BACK) {
    return VisibilityType::INVISIBLE;
  }

  return VisibilityType::VISIBLE;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief checks if another document revision (identified by its _from and _to 
/// values) is visible to the current transaction for read-only purposes
////////////////////////////////////////////////////////////////////////////////
         
bool Transaction::isVisibleForRead (TransactionId const& from, 
                                    TransactionId const& to) const {
  if (visibility(from) != Transaction::VisibilityType::VISIBLE) {
    return false;
  }

  if (visibility(to) == Transaction::VisibilityType::VISIBLE) {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the status of a sub-transaction of us
/// will only be called if sub is an iterated sub-transaction of us and we
/// are still ongoing
////////////////////////////////////////////////////////////////////////////////
      
Transaction::StatusType Transaction::statusSubTransaction (TransactionId const& sub) const {
  if (_committedSubTransactions.find(sub.own()) != _committedSubTransactions.end()) {
    // sub transaction has committed
    return Transaction::StatusType::COMMITTED;
  }

  if (hasOngoingSubTransaction()) {
    return Transaction::StatusType::ONGOING;
  }

  return Transaction::StatusType::ROLLED_BACK;
}

// -----------------------------------------------------------------------------
// --SECTION--                                          non-class friend methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief append the transaction to an output stream
////////////////////////////////////////////////////////////////////////////////

namespace triagens {
  namespace mvcc {

     std::ostream& operator<< (std::ostream& stream,
                               Transaction const* transaction) {
       stream << transaction->toString();
       return stream;
     }
     
     std::ostream& operator<< (std::ostream& stream,
                               Transaction const& transaction) {
       stream << transaction.toString();
       return stream;
     }

     std::ostream& operator<< (std::ostream& stream,
                               Transaction::VisibilityType visibility) {
       switch (visibility) {
        case Transaction::VisibilityType::INVISIBLE:
          stream << "INVISIBILE";
          break;
        case Transaction::VisibilityType::CONCURRENT:
          stream << "CONCURRENT";
          break;
        case Transaction::VisibilityType::VISIBLE:
          stream << "VISIBLE";
          break;
       }

       return stream;
     }

     std::ostream& operator<< (std::ostream& stream,
                               Transaction::StatusType status) {
       switch (status) {
        case Transaction::StatusType::ONGOING:
          stream << "ONGOING";
          break;
        case Transaction::StatusType::COMMITTED:
          stream << "COMMITTED";
          break;
        case Transaction::StatusType::ROLLED_BACK:
          stream << "ROLLED BACK";
          break;
       }

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
