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

LocalTransactionManagerCleanupThread::LocalTransactionManagerCleanupThread (LocalTransactionManager* manager)
  : Thread("transaction-manager"),
    _manager(manager),
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
    if (_manager->killRunningTransactions(5)) {
      _manager->deleteKilledTransactions();
    }
    else {
      usleep(1000 * 200);
    }
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                     class LocalTransactionManager
// -----------------------------------------------------------------------------

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

  _cleanupThread = new LocalTransactionManagerCleanupThread(this);

  if (! _cleanupThread->init() || ! _cleanupThread->start()) {
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_INTERNAL, "could not start cleanup thread");
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the transaction manager
////////////////////////////////////////////////////////////////////////////////

LocalTransactionManager::~LocalTransactionManager () {
  // TODO: abort all open transactions on shutdown!
  if (_cleanupThread) {
    _cleanupThread->join();
  }

  delete _cleanupThread;

  try {
    killRunningTransactions(0.0);
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

Transaction* LocalTransactionManager::createTransaction (TRI_vocbase_t* vocbase) {
  std::unique_ptr<Transaction> transaction(new TopLevelTransaction(this, nextId(), vocbase));

  // we do have a transaction now

  // insert into list of currently running transactions  
  insertRunningTransaction(transaction.get());

  return transaction.release();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a sub-transaction and lease it
////////////////////////////////////////////////////////////////////////////////

Transaction* LocalTransactionManager::createSubTransaction (TRI_vocbase_t* vocbase,
                                                            Transaction* parent) {
  TRI_ASSERT(parent != nullptr);
  TRI_ASSERT(vocbase == parent->vocbase());

  std::unique_ptr<Transaction> transaction(new SubTransaction(parent));

  // we do have a transaction now

  // insert into list of currently running transactions  
  insertRunningTransaction(transaction.get());

  return transaction.release();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief lease a transaction
////////////////////////////////////////////////////////////////////////////////

Transaction* LocalTransactionManager::leaseTransaction (TransactionId::IdType id) {
  WRITE_LOCKER(_lock);

  auto it = _runningTransactions.find(id);

  if (it == _runningTransactions.end()) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_TRANSACTION_NOT_FOUND);
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
  auto id = transaction->id()();

  WRITE_LOCKER(_lock);
  _leasedTransactions.erase(id);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief commit a transaction
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManager::commitTransaction (TransactionId::IdType id) {
  Transaction* transaction = nullptr;
    
  {
    WRITE_LOCKER(_lock);

    auto it = _runningTransactions.find(id);

    if (it == _runningTransactions.end()) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_TRANSACTION_NOT_FOUND);
    }

    transaction = (*it).second;
  }

  // TODO: assert that the transaction does not have any active subtransactions, or cancel them
  transaction->commit();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief rollback a transaction
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManager::rollbackTransaction (TransactionId::IdType id) {
  Transaction* transaction = nullptr;

  {
    WRITE_LOCKER(_lock);

    auto it = _runningTransactions.find(id);

    if (it == _runningTransactions.end()) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_TRANSACTION_NOT_FOUND);
    }

    transaction = (*it).second;
  }
  
  // TODO: assert that the transaction does not have any active subtransactions, or cancel them
  transaction->rollback();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief kill a transaction
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManager::killTransaction (TransactionId::IdType id) {
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
        result.emplace_back(TransactionInfo(it.second->id()(), it.second->startTime()));
      }
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief initializes a transaction with state
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManager::initializeTransaction (Transaction* transaction) {
  TRI_ASSERT(transaction->isOngoing());

  if (transaction->isTopLevel()) {
    // copy the list of currently running transactions into the transaction
    READ_LOCKER(_lock);
    static_cast<TopLevelTransaction*>(transaction)->setStartState(_runningTransactions);
  }

  transaction->_flags.initialized();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the status of a transaction
////////////////////////////////////////////////////////////////////////////////

Transaction::StatusType LocalTransactionManager::statusTransaction (TransactionId::IdType id) {
  return Transaction::StatusType::COMMITTED;  // FIXME: do something sensible
}

////////////////////////////////////////////////////////////////////////////////
/// @brief remove the transaction from the list of running transactions
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManager::removeRunningTransaction (Transaction* transaction,
                                                        bool transactionContainsModification) {
  TRI_ASSERT(! transaction->isOngoing());

  auto id = transaction->id()();

  WRITE_LOCKER(_lock); 

  if (transactionContainsModification) {
    if (transaction->status() == Transaction::StatusType::ROLLED_BACK) {
      // if this fails and throws, no harm will be done
      _failedTransactions.insert(id);
    }

    for (auto it : transaction->_subTransactions) {
      if (it.second == Transaction::StatusType::ROLLED_BACK) {
        // TODO: implement proper cleanup if this fails somewhere in the middle
        _failedTransactions.insert(id);
      }
    }
  }
  
  // delete from both lists
  _runningTransactions.erase(id);
  _leasedTransactions.erase(id);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief increment the transaction id counter and return it
////////////////////////////////////////////////////////////////////////////////

TransactionId LocalTransactionManager::nextId () {
  // TODO: top-level transactions must have the lowest 8 bits of their id
  // set to 0. FIXME
  return TransactionId(TransactionId::MainPart(TRI_NewTickServer()));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief insert the transaction into the list of running transactions
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManager::insertRunningTransaction (Transaction* transaction) {
  auto id = transaction->id()();

  WRITE_LOCKER(_lock); 
  auto it = _runningTransactions.emplace(std::make_pair(id, transaction));

  // must have inserted!
  TRI_ASSERT_EXPENSIVE(it.second);

  try {
    // if inserting into second list fails, we must rollback the insert!
    _leasedTransactions.emplace(id);
  }
  catch (...) {
    _runningTransactions.erase(it.second);
    throw;
  }
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
      if (it.second->startTime() + maxAge < now) {
        it.second->killed(true);
        found = true;
      }
    }
  }

  return found;
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
    std::unordered_map<TransactionId::IdType, Transaction*> runningTransactions(_runningTransactions);
    
    for (auto it : runningTransactions) {
      auto transaction = it.second;

      if (transaction->killed()) {
        // we found a killed transaction
        auto id = transaction->id()();

        auto it2 = _leasedTransactions.find(id);
  
        if (it2 == _leasedTransactions.end()) {
          // the transaction is killed and not currently leased
          // this means it is a candidate for deletion
           
          // remove from list of currently running transactions so it cannot
          // be found anymore
          runningTransactions.erase(id);

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
