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
#include "Mvcc/SubTransaction.h"
#include "Mvcc/TopLevelTransaction.h"
#include "Utils/Exception.h"
#include "VocBase/vocbase.h"

using namespace triagens::mvcc;

thread_local std::vector<Transaction*> LocalTransactionManager::_threadTransactions;

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create the transaction manager
////////////////////////////////////////////////////////////////////////////////

LocalTransactionManager::LocalTransactionManager (Transaction::IdType lastUsedId) 
  : TransactionManager(),
    _lastUsedId(lastUsedId) {
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
/// @brief create a top-level transaction
////////////////////////////////////////////////////////////////////////////////

TopLevelTransaction* LocalTransactionManager::createTopLevelTransaction (TRI_vocbase_t* vocbase) {
  // TODO
  TRI_ASSERT(false); 
  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a (potentially nested) transaction
////////////////////////////////////////////////////////////////////////////////

Transaction* LocalTransactionManager::createTransaction (TRI_vocbase_t* vocbase) {
  Transaction* transaction = nullptr;

  if (_threadTransactions.empty()) {
    // no other transactions present in thread

    // now create a new top-level transaction
    transaction = new TopLevelTransaction(this, Transaction::TransactionId(nextId(), 0), vocbase);
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

    transaction = new SubTransaction(parent);
  }

  // we must have a transaction now
  TRI_ASSERT(transaction != nullptr);

  try {
    _threadTransactions.push_back(transaction);
  }
  catch (...) {
    // if we got here, we have run out of memory
    // prevent a mem-leak
    delete transaction;
    throw;
  }

  transaction->registeredOnStack(true);

  return transaction;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief unregister a transaction
////////////////////////////////////////////////////////////////////////////////

void LocalTransactionManager::unregisterTransaction (Transaction* transaction) {
  // transaction must have already finished
  TRI_ASSERT(! transaction->isOngoing());

  if (transaction->registeredOnStack()) {
    // remove the transaction from the stack
    TRI_ASSERT(! _threadTransactions.empty());

    // the transaction that is unregistered must be at the top of the stack
    TRI_ASSERT(_threadTransactions.back() == transaction);
    _threadTransactions.pop_back();
  
    transaction->registeredOnStack(false);
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief increment the id counter and return the new id
////////////////////////////////////////////////////////////////////////////////

Transaction::IdType LocalTransactionManager::nextId () {
  return ++_lastUsedId;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
