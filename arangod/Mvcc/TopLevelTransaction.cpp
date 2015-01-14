////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC top-level transaction class
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

#include "TopLevelTransaction.h"
#include "Basics/logging.h"
#include "Mvcc/TransactionManager.h"
#include "Utils/Exception.h"
#include "VocBase/vocbase.h"

using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                         class TopLevelTransaction
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new top-level transaction
////////////////////////////////////////////////////////////////////////////////

TopLevelTransaction::TopLevelTransaction (TransactionManager* transactionManager,
                                          TransactionId const& id,
                                          TRI_vocbase_t* vocbase)
  : Transaction(transactionManager, id, vocbase),
    _runningTransactions(nullptr),
    _lastUsedSubId(0) {

  TRI_ASSERT_EXPENSIVE(id.mainPart() == id());
  TRI_ASSERT_EXPENSIVE(id.sequencePart() == 0);
  LOG_TRACE("creating %s", toString().c_str());
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the transaction
////////////////////////////////////////////////////////////////////////////////

TopLevelTransaction::~TopLevelTransaction () {
  LOG_TRACE("destroying %s", toString().c_str());

  if (_status == Transaction::StatusType::ONGOING) {
    rollback();
  }

  if (_runningTransactions != nullptr) {
    delete _runningTransactions;
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief get a string representation of the transaction
////////////////////////////////////////////////////////////////////////////////

std::string TopLevelTransaction::toString () const {
  std::string result("TopLevelTransaction ");
  result += std::to_string(_id());

  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief provide the next id for a sub-transaction
////////////////////////////////////////////////////////////////////////////////

TransactionId TopLevelTransaction::nextSubId () {
  return TransactionId(_id(), ++_lastUsedSubId);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief sets the start state (e.g. list of running transactions
////////////////////////////////////////////////////////////////////////////////

void TopLevelTransaction::setStartState (std::unordered_set<TransactionId> const& transactions) {
  TRI_ASSERT(_runningTransactions == nullptr);

  if (! transactions.empty()) {
    _runningTransactions = new std::unordered_set<TransactionId>(transactions.begin(), transactions.end());
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
