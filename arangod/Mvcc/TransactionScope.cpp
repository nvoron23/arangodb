////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC transaction scope class
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

#include "TransactionScope.h"
#include "Mvcc/Transaction.h"
#include "Mvcc/TransactionManager.h"
#include "Utils/Exception.h"
#include "VocBase/vocbase.h"

using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                            class TransactionScope
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                            thread local variables
// -----------------------------------------------------------------------------

thread_local std::vector<Transaction*> TransactionScope::_threadTransactions;

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief join an existing transaction in an outer scope or a new transaction, 
/// which will be automatically freed when the scope is left
////////////////////////////////////////////////////////////////////////////////

TransactionScope::TransactionScope (TRI_vocbase_t* vocbase,
                                    bool allowNesting)
  : _transactionManager(triagens::mvcc::TransactionManager::instance()),
    _transaction(nullptr),
    _isOur(false),
    _pushedOnThreadStack(false) {

  if (! allowNesting || _threadTransactions.empty()) {
    // start our own transaction
    _transaction = _transactionManager->createTransaction(vocbase);
    _isOur = true;
      
    if (allowNesting) {
      pushOnThreadStack(_transaction);
      _pushedOnThreadStack = true;
    }
  }
  else {
    // reuse an existing transaction
    auto existing = _threadTransactions.back();
    TRI_ASSERT(existing != nullptr);
    
    // check if the database is still the same
    if (vocbase != existing->vocbase()) {
      THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_TRANSACTION_INTERNAL, "cannot change database for nested transaction");
    }
    
    _transaction = existing;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the transaction scope
/// if the transaction was created by the scope, it will abort the transaction
/// and destroy it
////////////////////////////////////////////////////////////////////////////////

TransactionScope::~TransactionScope () {
  if (_isOur) {
    TRI_ASSERT(_transaction != nullptr);
    if (_pushedOnThreadStack) {
      // remove the transaction from the stack
      popFromThreadStack(_transaction);
    }
    try {
      delete _transaction;
    }
    catch (...) {
      // destructor must not fail
    }
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief commit the scoped transaction
/// this will do nothing if the transaction was re-used from an outer scope,
/// but will commit the transaction otherwise
////////////////////////////////////////////////////////////////////////////////

void TransactionScope::commit () {
  if (_isOur) {
    _transaction->commit();
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief push the transaction onto the thread-local stack
////////////////////////////////////////////////////////////////////////////////

void TransactionScope::pushOnThreadStack (Transaction* transaction) {
  _threadTransactions.push_back(transaction);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief pop the transaction from the thread-local stack
////////////////////////////////////////////////////////////////////////////////

void TransactionScope::popFromThreadStack (Transaction* transaction) {
  TRI_ASSERT(! _threadTransactions.empty());

  // the transaction that is popped must be at the top of the stack
  TRI_ASSERT(_threadTransactions.back() == transaction);
  _threadTransactions.pop_back();
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
