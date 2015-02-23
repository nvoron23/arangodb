////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC transaction manager, local
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

#include "LocalTransactionManager.h"
#include "Basics/ReadLocker.h"
#include "Basics/WriteLocker.h"
#include "Mvcc/SubTransaction.h"
#include "Mvcc/TopLevelTransaction.h"
#include "Utils/Exception.h"
#include "VocBase/server.h"
#include "VocBase/vocbase.h"

using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                              LocalTransactionManagerCleanupThread
// -----------------------------------------------------------------------------

LocalTransactionManagerCleanupThread::LocalTransactionManagerCleanupThread (LocalTransactionManager* manager,
                                                                            double defaultTtl)
  : Thread("transaction-manager"),
    _manager(manager),
    _defaultTtl(defaultTtl),
    _stopped(false) {
}

LocalTransactionManagerCleanupThread::~LocalTransactionManagerCleanupThread () {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief initialize the thread
////////////////////////////////////////////////////////////////////////////////

bool LocalTransactionManagerCleanupThread::init () {
  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief stop the thread
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManagerCleanupThread::stop () {
  _stopped = true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief thread main loop
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManagerCleanupThread::run () {
  while (! _stopped) {
    bool sleep = true;

    try {
      if (_manager->killRunningTransactions(_defaultTtl)) {
        _manager->deleteKilledTransactions();
        sleep = false;
      }
    }
    catch (...) {
    }
    
    if (sleep) {
      usleep(1000 * 200);
    }
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                     class LocalTransactionManager
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief default transaction expiration time
////////////////////////////////////////////////////////////////////////////////

double const LocalTransactionManager::DefaultTtl = 5.0;

////////////////////////////////////////////////////////////////////////////////
/// @brief reserve room to keep in list of transactions for failed transactions
////////////////////////////////////////////////////////////////////////////////

size_t const LocalTransactionManager::ReserveRoomForFailed = 16;

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create the transaction manager
////////////////////////////////////////////////////////////////////////////////

LocalTransactionManager::LocalTransactionManager () 
  : TransactionManager(),
    _lock(),
    _runningTransactions(),
    _failedTransactions(),
    _leasedTransactions(),
    _cleanupThread(nullptr) {

  _cleanupThread = new LocalTransactionManagerCleanupThread(this, DefaultTtl);

  if (! _cleanupThread->init() || ! _cleanupThread->start()) {
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_INTERNAL, "could not start cleanup thread");
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the transaction manager
////////////////////////////////////////////////////////////////////////////////

LocalTransactionManager::~LocalTransactionManager () {
  if (_cleanupThread) {
    _cleanupThread->stop();
    _cleanupThread->join();
  }

  delete _cleanupThread;

  // abort all open transactions on shutdown!
  try {
    killAllRunningTransactions();
    deleteKilledTransactions();
  }
  catch (...) {
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create a top-level transaction and lease it
////////////////////////////////////////////////////////////////////////////////

Transaction* LocalTransactionManager::createTransaction (TRI_vocbase_t* vocbase,
                                                         std::map<std::string, bool> const& collections,
                                                         double ttl) {
  std::unique_ptr<Transaction> transaction(new TopLevelTransaction(this, nextId(), vocbase, collections, ttl));

  // we do have a transaction now

  // insert into list of currently running transactions  
  insertRunningTransaction(transaction.get());

  return transaction.release();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a sub-transaction and lease it
////////////////////////////////////////////////////////////////////////////////

Transaction* LocalTransactionManager::createSubTransaction (TRI_vocbase_t* vocbase,
                                                            Transaction* parent,
                                                            std::map<std::string, bool> const& collections,
                                                            double ttl) {
  TRI_ASSERT(parent != nullptr);
  TRI_ASSERT(vocbase == parent->vocbase());

  std::unique_ptr<Transaction> transaction(new SubTransaction(parent, collections, ttl));

  // we do have a transaction now

  // insert into list of currently running transactions
  // this might fail 
  insertRunningTransaction(transaction.get());

  // only if this succeeds we can modify the parent transaction
  parent->_ongoingSubTransaction = transaction.get();

  return transaction.release();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief lease a transaction
////////////////////////////////////////////////////////////////////////////////

Transaction* LocalTransactionManager::leaseTransaction (TransactionId::InternalType id) {
  WRITE_LOCKER(_lock);

  auto it = _runningTransactions.find(id);

  if (it == _runningTransactions.end()) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_TRANSACTION_NOT_FOUND);
  }
  
  TRI_IF_FAILURE("LocalTransactionManager::leaseTransaction") {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
  }

  auto found = _leasedTransactions.insert(id);

  if (! found.second) {
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_INTERNAL, "trying to lease an already leased transaction");
  }

  return (*it).second;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief unlease a transaction
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManager::unleaseTransaction (Transaction* transaction) {
  auto id = transaction->id();

  WRITE_LOCKER(_lock);
  _leasedTransactions.erase(id.own());
}

////////////////////////////////////////////////////////////////////////////////
/// @brief commit a transaction
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManager::commitTransaction (TransactionId::InternalType id) {
  Transaction* transaction = nullptr;
    
  {
    WRITE_LOCKER(_lock);

    auto it = _runningTransactions.find(id);

    if (it == _runningTransactions.end()) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_TRANSACTION_NOT_FOUND);
    }

    transaction = (*it).second;
  }

  transaction->commit();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief rollback a transaction
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManager::rollbackTransaction (TransactionId::InternalType id) {
  Transaction* transaction = nullptr;

  {
    WRITE_LOCKER(_lock);

    auto it = _runningTransactions.find(id);

    if (it == _runningTransactions.end()) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_TRANSACTION_NOT_FOUND);
    }

    transaction = (*it).second;
  }
  
  transaction->rollback();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief kill a transaction
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManager::killTransaction (TransactionId::InternalType id) {
  READ_LOCKER(_lock);

  auto it = _runningTransactions.find(id);

  if (it == _runningTransactions.end()) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_TRANSACTION_NOT_FOUND);
  }

  auto transaction = (*it).second;
  // still hold the lock while calling killed()
  // this ensures that the transaction object cannot be invalidated by other threads
  // while we are using it
  transaction->killed(true);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief get a list of all running transactions
////////////////////////////////////////////////////////////////////////////////

std::vector<TransactionInfo> LocalTransactionManager::runningTransactions (TRI_vocbase_t* vocbase) {
  std::vector<TransactionInfo> result;
  // already allocate some space
  result.reserve(32);

  {
    READ_LOCKER(_lock);

    for (auto const& it : _runningTransactions) {
      if (vocbase == nullptr || vocbase == it.second->vocbase()) {
        result.emplace_back(TransactionInfo(it.second->id(), it.second->startTime()));
      }
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the status of a transaction
////////////////////////////////////////////////////////////////////////////////

Transaction::StatusType LocalTransactionManager::statusTransaction (TransactionId const& id) const {
  READ_LOCKER(_lock); 

  if (_failedTransactions.find(id.own()) != _failedTransactions.end()) {
    // transaction has failed
    return Transaction::StatusType::ROLLED_BACK;
  }
  
  if (_runningTransactions.find(id.own()) != _runningTransactions.end()) {
    // transaction is still running
    return Transaction::StatusType::ONGOING;
  }

  return Transaction::StatusType::COMMITTED;  
}

////////////////////////////////////////////////////////////////////////////////
/// @brief adds the transactions from the parameter to the list of failed
/// transactions
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManager::addFailedTransactions (std::unordered_set<TransactionId::InternalType> const& transactions) {
  TRI_IF_FAILURE("LocalTransactionManager::addFailedTransactions") {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
  }

  WRITE_LOCKER(_lock); 

  _failedTransactions.insert(transactions.begin(), transactions.end());
}

////////////////////////////////////////////////////////////////////////////////
/// @brief removes the transaction from the list of running transactions and
/// deletes the transaction object
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManager::deleteRunningTransaction (Transaction* transaction) {
  TRI_ASSERT(! transaction->isOngoing());

  auto id = transaction->id();

  {
    WRITE_LOCKER(_lock); 

    if (transaction->status() == Transaction::StatusType::ROLLED_BACK) {
      // if this fails and throws, no harm will be done
      _failedTransactions.emplace(id.own());
    }
  
    // delete from both lists
    _runningTransactions.erase(id.own());
    _leasedTransactions.erase(id.own());
  
    // must still hold this lock for deletion
    delete transaction;
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief initializes a transaction with state
/// this is called under the write-lock of the transaction manager
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManager::initializeTransaction (Transaction* transaction) {
  TRI_ASSERT(transaction->isOngoing());

  if (transaction->isTopLevel()) {
    std::unique_ptr<std::unordered_set<TransactionId::InternalType>> runningTransactions(new std::unordered_set<TransactionId::InternalType>());
    auto set = runningTransactions.get();

    set->reserve(16);

    {
      // copy the list of currently running transactions into the transaction
      set->reserve(_runningTransactions.size());

      for (auto it : _runningTransactions) {
        set->emplace(it.first);
      }
    }

    // outside the lock
    if (! runningTransactions.get()->empty()) {
      static_cast<TopLevelTransaction*>(transaction)->setStartState(runningTransactions);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief increment the transaction id counter and return it
////////////////////////////////////////////////////////////////////////////////

TransactionId LocalTransactionManager::nextId () {
  auto id = TRI_NewTickServer();
 return TransactionId(id, id);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief insert the transaction into the list of running transactions
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManager::insertRunningTransaction (Transaction* transaction) {
  auto id = transaction->id();

  WRITE_LOCKER(_lock); 

  TRI_IF_FAILURE("LocalTransactionManager::insertRunningTransaction1") {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
  }

  // must have room in the list of _failedTransactions if this transaction later fails
  _failedTransactions.reserve(_failedTransactions.size() + ReserveRoomForFailed);

  auto it = _runningTransactions.emplace(std::make_pair(id.own(), transaction));

  // must have inserted!
  TRI_ASSERT_EXPENSIVE(it.second);

  try {
    TRI_IF_FAILURE("LocalTransactionManager::insertRunningTransaction2") {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
    }

    // if inserting into second list fails, we must rollback the insert!
    _leasedTransactions.emplace(id.own());
  }
  catch (...) {
    _runningTransactions.erase(it.first);
  
    // must be present in none
    TRI_ASSERT_EXPENSIVE(_runningTransactions.find(id.own()) == _runningTransactions.end());
    TRI_ASSERT_EXPENSIVE(_leasedTransactions.find(id.own()) == _leasedTransactions.end());
    throw;
  }

  // must be present in both
  TRI_ASSERT_EXPENSIVE(_runningTransactions.find(id.own()) != _runningTransactions.end());
  TRI_ASSERT_EXPENSIVE(_leasedTransactions.find(id.own()) != _leasedTransactions.end());

  // still holding the write-lock here
  initializeTransaction(transaction);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief kill long-running transactions if they are older than maxAge
/// seconds. this only sets the transactions' kill bit, but does not physically
/// remove them
////////////////////////////////////////////////////////////////////////////////

bool LocalTransactionManager::killRunningTransactions (double maxAge) {
  double const now = TRI_microtime();
  bool found = false;

  {
    READ_LOCKER(_lock);

    for (auto it : _runningTransactions) {
      // kill a transaction if its expiration time has come
      // or, if this is not set, after the specified amount of seconds
      auto expireTime = it.second->expireTime();

      if ((expireTime > 0.0 && expireTime < now) ||
          (expireTime <= 0.0 && it.second->startTime() + maxAge < now)) {
        if (! it.second->killed()) {
          it.second->killed(true);
          found = true;
        }
      }
    }
  }

  return found;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief unconditionally kill all running transactions 
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManager::killAllRunningTransactions () {
  READ_LOCKER(_lock);

  for (auto it : _runningTransactions) {
    if (! it.second->killed()) {
      it.second->killed(true);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief physically remove killed transactions
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManager::deleteKilledTransactions () {
  std::vector<Transaction*> transactionsToDelete;
  transactionsToDelete.reserve(32);

  {
    WRITE_LOCKER(_lock);
    // create a full copy of the current state in local variables
    // so we can safely swap later
    std::unordered_map<TransactionId::InternalType, Transaction*> runningTransactions(_runningTransactions);
    
    for (auto it : runningTransactions) {
      auto transaction = it.second;

      if (transaction->killed()) {
        // we found a killed transaction
        auto id = transaction->id();

        auto it2 = _leasedTransactions.find(id.own());
  
        if (it2 == _leasedTransactions.end()) {
          // the transaction is killed and not currently leased
          // this means it is a candidate for deletion
           
          // remove from list of currently running transactions so it cannot
          // be found anymore
          runningTransactions.erase(id.own());
  
          TRI_IF_FAILURE("LocalTransactionManager::deleteKilledTransactions") {
            THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
          }

          transactionsToDelete.emplace_back(transaction);
        }
      }
    }

    // we're done with modifications to the local variables
    // now do a swap
    _runningTransactions = runningTransactions;
  }

  // now the transactions we want to delete are not present in neither
  // _runningTransactions nor _leasedTransactions
  // no one else can access them, and we can delete them without holding the lock

  for (auto it : transactionsToDelete) {
    try {
      delete it;
    }
    catch (...) {
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
