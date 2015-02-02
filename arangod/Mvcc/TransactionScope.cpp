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
#include "Mvcc/TransactionStackAccessor.h"
#include "Utils/Exception.h"
#include "VocBase/vocbase.h"

using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                            class TransactionScope
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief join an existing transaction in an outer scope or a new transaction, 
/// which will be automatically freed when the scope is left
////////////////////////////////////////////////////////////////////////////////

TransactionScope::TransactionScope (TRI_vocbase_t* vocbase,
                                    bool forceNew,
                                    bool canBeSubTransaction) 
  : _transactionManager(triagens::mvcc::TransactionManager::instance()),
    _transaction(nullptr),
    _isOur(false),
    _pushedOnThreadStack(false) {

  TransactionStackAccessor accessor;
  bool const hasThreadTransactions = ! accessor.isEmpty();

  if (! hasThreadTransactions) {
    // if there are no other transactions present in the thread, we
    // definitely have to start our own
    forceNew = true;
  }
  
  if (forceNew) {
    std::cout << "CREATING TRANSACTION IN SCOPE\n";
    // start our own transaction
    if (canBeSubTransaction && hasThreadTransactions) {
      // start a sub transaction
      auto parent = accessor.peek();
      TRI_ASSERT(parent != nullptr);

      _transaction = _transactionManager->createSubTransaction(vocbase, parent);
    }
    else {
      // start a top-level transaction
      _transaction = _transactionManager->createTransaction(vocbase);
    }
    _isOur = true;
    
    // push transaction on the stack 
    accessor.push(_transaction); 

    _pushedOnThreadStack = true;
  }
  else {
    // reuse an existing transaction
    auto existing = accessor.peek();
    TRI_ASSERT(existing != nullptr);
    
    // check if the database is still the same
    if (vocbase != existing->vocbase()) {
      THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_TRANSACTION_INTERNAL, "cannot change database for nested operation");
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
    std::cout << "DESTROYING TRANSACTION CREATED IN SCOPE\n";
    TRI_ASSERT(_transaction != nullptr);
    if (_pushedOnThreadStack) {
      // remove the transaction from the stack
      TransactionStackAccessor accessor;
      auto popped = accessor.pop();
      TRI_ASSERT(popped == _transaction);
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
    std::cout << "COMMITTING TRANSACTION CREATED IN SCOPE\n";
    _transaction->commit();
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
