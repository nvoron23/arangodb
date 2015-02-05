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
    _flags(),
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
/// @brief update the number of inserted documents
////////////////////////////////////////////////////////////////////////////////

void Transaction::incNumInserted (TransactionCollection const* collection,
                                  bool waitForSync) {
  auto cid = collection->id();
  auto it = _stats.find(cid);

  if (it == _stats.end()) {
    CollectionStats stats;
    stats.numInserted++;
    stats.waitForSync = waitForSync || collection->waitForSync();
    _stats.insert(std::make_pair(cid, stats));
  }
  else {
    (*it).second.numInserted++;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief update the number of removed documents
////////////////////////////////////////////////////////////////////////////////

void Transaction::incNumRemoved (TransactionCollection const* collection,
                                 bool waitForSync) {
  auto cid = collection->id();
  auto it = _stats.find(cid);

  if (it == _stats.end()) {
    CollectionStats stats;
    stats.numRemoved++;
    stats.waitForSync = waitForSync || collection->waitForSync();
    _stats.insert(std::make_pair(cid, stats));
  }
  else {
    (*it).second.numRemoved++;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief update the number of removed documents
////////////////////////////////////////////////////////////////////////////////

        void incNumRemoved (TransactionCollection const*);


////////////////////////////////////////////////////////////////////////////////
/// @brief return the transaction status
////////////////////////////////////////////////////////////////////////////////

Transaction::StatusType Transaction::status () const {
  // TODO: implement locking here in case multiple threads access the same transaction
  return _status;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief set the killed flag
////////////////////////////////////////////////////////////////////////////////

void Transaction::killed (bool) {
  _killed = true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief check the killed flag
////////////////////////////////////////////////////////////////////////////////

bool Transaction::killed () {
  return _killed;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                 protected methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief initialize and fetch the current state from the transaction manager
////////////////////////////////////////////////////////////////////////////////

void Transaction::initializeState () {
  _transactionManager->initializeTransaction(this);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief this is called when a subtransaction is finished
////////////////////////////////////////////////////////////////////////////////

void Transaction::subTransactionFinished (Transaction* transaction) {
  auto const id = transaction->id();
  auto const status = transaction->status();

  if (status == StatusType::COMMITTED) { 
    bool hasAnyOperation = false;

    // sub-transaction has committed, now copy its stats into our stats
    for (auto const& it : transaction->_stats) {
      if (! it.second.hasOperations()) {
        continue;
      }

      hasAnyOperation = true;

      auto it2 = _stats.find(it.first);
      if (it2 != _stats.end()) {
        // merge our own stats with the sub-transaction's stats
        (*it2).second.merge(it.second);
      }
      else {
        // take over the stats from the sub-transaction
        _stats.emplace(std::make_pair(it.first, it.second));
      }
    }
    
    // track the id of the transaction, but only if the transaction has actually
    // modified data
    if (hasAnyOperation) { 
      _committedSubTransactions.emplace(id.own());
    }
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
    
Transaction::VisibilityType Transaction::visibility (TransactionId const& other) {
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
                                    TransactionId const& to) {
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
      
Transaction::StatusType Transaction::statusSubTransaction (TransactionId const& sub) {
  if (_committedSubTransactions.find(sub.own()) != _committedSubTransactions.end()) {
    // sub transaction has committed
    return Transaction::StatusType::COMMITTED;
  }

  if (hasOngoingSubTransaction()) {
    return Transaction::StatusType::ONGOING;
  }

  // we should never get here
  TRI_ASSERT(false);
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

  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
