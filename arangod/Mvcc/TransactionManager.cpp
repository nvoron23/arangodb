////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC transaction manager
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

#include "TransactionManager.h"
#include "Basics/logging.h"
#include "Mvcc/LocalTransactionManager.h"
#include "Cluster/ServerState.h"

using namespace triagens::mvcc;

////////////////////////////////////////////////////////////////////////////////
/// @brief the global transaction manager instance
////////////////////////////////////////////////////////////////////////////////

TransactionManager* TransactionManager::Instance = nullptr;

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create the transaction manager
////////////////////////////////////////////////////////////////////////////////

TransactionManager::TransactionManager () {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the transaction manager
////////////////////////////////////////////////////////////////////////////////

TransactionManager::~TransactionManager () {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief initialize the transaction manager
/// TODO: this is a stub method and needs to be implemented
////////////////////////////////////////////////////////////////////////////////

void TransactionManager::initialize () {
  LOG_TRACE("initializing transaction manager");

  TRI_ASSERT(Instance == nullptr);
  if (triagens::arango::ServerState::instance()->isCoordinator()) {
    // TODO
    Instance = nullptr;
    TRI_ASSERT(false);
  }
  else {
    // TODO
    Instance = new LocalTransactionManager();
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief shutdown the transaction manager
////////////////////////////////////////////////////////////////////////////////

void TransactionManager::shutdown () {
  LOG_TRACE("shutting down transaction manager");

  delete Instance;
  Instance = nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the global transaction manager instance
////////////////////////////////////////////////////////////////////////////////

TransactionManager* TransactionManager::instance () {
  TRI_ASSERT_EXPENSIVE(Instance != nullptr);
  return Instance;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
