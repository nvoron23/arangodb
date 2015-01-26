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

Transaction* LocalTransactionManager::createTransaction (TRI_vocbase_t* vocbase) {
  std::unique_ptr<Transaction> transaction(new TopLevelTransaction(this, nextId(), vocbase));

  // we do have a transaction now

  // insert into list of currently running transactions  
  insertRunningTransaction(transaction.get());

  return transaction.release();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief unregister a transaction
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManager::unregisterTransaction (Transaction* transaction) {
  // transaction must have already finished
  TRI_ASSERT(! transaction->isOngoing());

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

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
