////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC transaction manager, distributed
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

#include "DistributedTransactionManager.h"
#include "Utils/Exception.h"
#include "VocBase/vocbase.h"

using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create the transaction manager
////////////////////////////////////////////////////////////////////////////////

DistributedTransactionManager::DistributedTransactionManager () 
  : TransactionManager() {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the transaction manager
////////////////////////////////////////////////////////////////////////////////

DistributedTransactionManager::~DistributedTransactionManager () {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a (potentially nested) transaction
////////////////////////////////////////////////////////////////////////////////

Transaction* DistributedTransactionManager::createTransaction (TRI_vocbase_t* vocbase) {
  // TODO
  TRI_ASSERT(false);
  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief unregister a transaction
////////////////////////////////////////////////////////////////////////////////

void DistributedTransactionManager::unregisterTransaction (Transaction* transaction) {
  // TODO
  TRI_ASSERT(! transaction->isOngoing());
  TRI_ASSERT(false);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief aborts a transaction
////////////////////////////////////////////////////////////////////////////////

void DistributedTransactionManager::abortTransaction (TransactionId::IdType id) {
  // TODO
  TRI_ASSERT(false);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief initializes a transaction with state
////////////////////////////////////////////////////////////////////////////////

void DistributedTransactionManager::initializeTransaction (Transaction* transaction) {
  // TODO
  TRI_ASSERT(transaction->isOngoing());
  TRI_ASSERT(false);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the status of a transaction
////////////////////////////////////////////////////////////////////////////////

Transaction::StatusType DistributedTransactionManager::statusTransaction (TransactionId::IdType) {
  // TODO
  return Transaction::StatusType::COMMITTED;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
