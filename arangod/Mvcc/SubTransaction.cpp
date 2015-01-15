////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC sub transaction class
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

#include "SubTransaction.h"
#include "Basics/logging.h"
#include "Mvcc/TopLevelTransaction.h"
#include "Mvcc/Transaction.h"
#include "Mvcc/TransactionCollection.h"
#include "Mvcc/TransactionManager.h"
#include "Utils/Exception.h"

using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                              class SubTransaction
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new sub transaction
////////////////////////////////////////////////////////////////////////////////

SubTransaction::SubTransaction (Transaction* parent)
  : Transaction(parent->transactionManager(), parent->topLevelTransaction()->nextSubId(), parent->vocbase()),
    _topLevelTransaction(parent->topLevelTransaction()),
    _parentTransaction(parent) {
 
  LOG_TRACE("creating %s", toString().c_str());
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the transaction
////////////////////////////////////////////////////////////////////////////////

SubTransaction::~SubTransaction () {
  if (_status == Transaction::StatusType::ONGOING) {
    try {
      rollback();
    }
    catch (...) {
      // destructors better not throw
    }
  }
  
  LOG_TRACE("destroying %s", toString().c_str());
}

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief returns a collection used in the transaction
/// this registers the collection in the transaction if not yet present
////////////////////////////////////////////////////////////////////////////////
        
TransactionCollection* SubTransaction::collection (std::string const& name) {
  return _topLevelTransaction->collection(name);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns a collection used in the transaction
/// this registers the collection in the transaction if not yet present
////////////////////////////////////////////////////////////////////////////////
        
TransactionCollection* SubTransaction::collection (TRI_voc_cid_t cid) {
  return _topLevelTransaction->collection(cid);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief get a string representation of the transaction
////////////////////////////////////////////////////////////////////////////////

std::string SubTransaction::toString () const {
  std::string result("SubTransaction ");
  result += std::to_string(_id());
  result += ", parent: ";
  result += _topLevelTransaction->toString();

  return result;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
