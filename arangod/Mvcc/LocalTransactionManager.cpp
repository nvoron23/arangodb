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

thread_local std::vector<Transaction*> LocalTransactionManager::_threadTransactions;

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create the transaction manager
////////////////////////////////////////////////////////////////////////////////

LocalTransactionManager::LocalTransactionManager () 
  : TransactionManager(),
    _lock(),
    _runningTransactions() {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the transaction manager
////////////////////////////////////////////////////////////////////////////////

LocalTransactionManager::~LocalTransactionManager () {
}

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create a (potentially nested) transaction
////////////////////////////////////////////////////////////////////////////////

Transaction* LocalTransactionManager::createTransaction (TRI_vocbase_t* vocbase,
                                                         bool forceTopLevel) {
  std::unique_ptr<Transaction> transaction;

  if (forceTopLevel || _threadTransactions.empty()) {
    // no other transactions present in thread

    // now create a new top-level transaction
    transaction.reset(new TopLevelTransaction(this, nextId(), vocbase));
  }
  else {
    // there is already a transaction present in our thread

    // determine the parent transaction
    auto parent = _threadTransactions.back();
    TRI_ASSERT(parent != nullptr);

    // check if the database is still the same
    if (vocbase != parent->vocbase()) {
      THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_TRANSACTION_INTERNAL, "cannot change database for sub transaction");
    }

    transaction.reset(new SubTransaction(parent));
  }

  // we do have a transaction now
  TRI_ASSERT(transaction.get() != nullptr);

  // insert into list of currently running transactions  
  insertRunningTransaction(transaction.get());


  // only push onto thread-local stack if the caller has not explicitly 
  // requested a top-level transaction
  if (! forceTopLevel) {
    try {
      // insert into thread-local list of transactions
      pushOnThreadStack(transaction.get());
    }
    catch (...) {
      // if we got here, we have run out of memory
      removeRunningTransaction(transaction.get());
      throw;
    }
  }

  return transaction.release();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief unregister a transaction
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManager::unregisterTransaction (Transaction* transaction) {
  // transaction must have already finished
  TRI_ASSERT(! transaction->isOngoing());

  if (transaction->_flags.pushedOnThreadStack()) {
    // remove the transaction from the stack
    popFromThreadStack(transaction);
  }

  removeRunningTransaction(transaction);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief aborts a transaction
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManager::abortTransaction (TransactionId const& id) {
  WRITE_LOCKER(_lock);
  auto it = _runningTransactions.find(id);
  if (it == _runningTransactions.end()) {
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_INTERNAL, "transaction not found"); // TODO: fix code and message
  }

  auto transaction = (*it).second;
  // still hold the write-lock while calling setAborted()
  // this ensures that the transaction object cannot be invalidated by other threads
  // while we are using it
  transaction->setAborted();
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

Transaction::StatusType LocalTransactionManager::statusTransaction (
                TransactionId id) {
  return Transaction::StatusType::COMMITTED;  // FIXME: do something sensible
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
  WRITE_LOCKER(_lock); 
  _runningTransactions.emplace(std::make_pair(transaction->id(), transaction));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief remove the transaction from the list of running transactions
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManager::removeRunningTransaction (Transaction* transaction) {
  WRITE_LOCKER(_lock); 
  _runningTransactions.erase(transaction->id());
}

////////////////////////////////////////////////////////////////////////////////
/// @brief push the transaction onto the thread-local stack
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManager::pushOnThreadStack (Transaction* transaction) {
  _threadTransactions.push_back(transaction);
  transaction->_flags.pushedOnThreadStack(true);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief pop the transaction from the thread-local stack
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManager::popFromThreadStack (Transaction* transaction) {
  TRI_ASSERT(! _threadTransactions.empty());

  // the transaction that is popped must be at the top of the stack
  TRI_ASSERT(_threadTransactions.back() == transaction);
  _threadTransactions.pop_back();
  
  transaction->_flags.pushedOnThreadStack(false);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
