////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC transaction class
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

#include "Transaction.h"
#include "Basics/logging.h"
#include "Basics/ReadLocker.h"
#include "Basics/WriteLocker.h"
#include "Mvcc/TransactionManager.h"
#include "Utils/Exception.h"
#include "VocBase/vocbase.h"

using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                                 class Transaction
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new transaction, with id provided by the transaction
/// manager
////////////////////////////////////////////////////////////////////////////////

Transaction::Transaction (TransactionManager* transactionManager,
                          TransactionId const& id,
                          TRI_vocbase_t* vocbase)
  : _transactionManager(transactionManager),
    _id(id),
    _vocbase(vocbase),
    _status(Transaction::StatusType::ONGOING),
    _flags(),
    _lock(),
    _aborted(false) {

  TRI_ASSERT(_transactionManager != nullptr);
  TRI_ASSERT(_vocbase != nullptr);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the transaction
////////////////////////////////////////////////////////////////////////////////

Transaction::~Transaction () {
  // note: automatic rollback is called from the destructors of the derived classes
}

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief return the transaction status
////////////////////////////////////////////////////////////////////////////////

Transaction::StatusType Transaction::status () const {
  // TODO: implement locking here in case multiple threads access the same transaction
  return _status;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief commit the transaction
////////////////////////////////////////////////////////////////////////////////

int Transaction::commit () {
  // TODO: implement locking here in case multiple threads access the same transaction
  LOG_TRACE("committing transaction %s", toString().c_str());

  if (_status != Transaction::StatusType::ONGOING) {
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_TRANSACTION_INTERNAL, "cannot commit finished transaction");
  }

  _status = StatusType::COMMITTED;
  _transactionManager->unregisterTransaction(this);

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief roll back the transaction
////////////////////////////////////////////////////////////////////////////////

int Transaction::rollback () {
  // TODO: implement locking here in case multiple threads access the same transaction
  LOG_TRACE("rolling back transaction %s", toString().c_str());

  if (_status != Transaction::StatusType::ONGOING) {
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_TRANSACTION_INTERNAL, "cannot rollback finished transaction");
  }

  _status = StatusType::ROLLED_BACK;
  _transactionManager->unregisterTransaction(this);

  return TRI_ERROR_NO_ERROR;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                 protected methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief initialize and fetch the current state from the transaction manager
////////////////////////////////////////////////////////////////////////////////

void Transaction::initializeState () {
  _transactionManager->initializeTransaction(this);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief set the aborted flag
////////////////////////////////////////////////////////////////////////////////

void Transaction::setAborted () {
  WRITE_LOCKER(_lock);
  _aborted = true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief check the aborted flag
////////////////////////////////////////////////////////////////////////////////

bool Transaction::isAborted () {
  READ_LOCKER(_lock);
  return _aborted;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief visibility, this implements the MVCC logic of what this transaction
/// can see, returns the visibility of the other transaction for this one.
/// The result can be INVISIBLE, INCONFLICT or VISIBLE. We guarantee
/// INVISIBLE < INCONFLICT < VISIBLE, such that one can do things like
/// "visibility(other) < VISIBLE" legally.
////////////////////////////////////////////////////////////////////////////////
    
Transaction::VisibilityType Transaction::visibility(TransactionId other) {
  if (_id.isSameTransaction(other())) {   // same top level transaction?
    if (other.sequencePart() > _id.sequencePart()) {
      return VisibilityType::INVISIBLE;
    }
    else if (other.sequencePart() < _id.sequencePart()) {
      return _transactionManager->statusTransaction(other) == 
             StatusType::COMMITTED
           ? VisibilityType::VISIBLE
           : VisibilityType::INVISIBLE;
    }
    else {
      return VisibilityType::VISIBLE;
    }
  }
  else {
    // Other top level transaction
    if (other.mainPart() > _id.mainPart()) {
      // Started after us
      return VisibilityType::INVISIBLE;
    }

    if (wasOngoingAtStart(other)) {
      return VisibilityType::CONCURRENT;
    }

    // Optimisation:
    if (isNotAborted(other)) {
      return VisibilityType::VISIBLE;
    }

    return _transactionManager->statusTransaction(other) == 
           StatusType::COMMITTED
         ? VisibilityType::VISIBLE
         : VisibilityType::INVISIBLE;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief wasOngoingAtStart, check whether or not another transaction was
/// ongoing when this one started
////////////////////////////////////////////////////////////////////////////////

bool Transaction::wasOngoingAtStart (TransactionId other) {
  return false;   // FIXME: do something sensible
}

// -----------------------------------------------------------------------------
// --SECTION--                                          non-class friend methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief append the transaction to an output stream
////////////////////////////////////////////////////////////////////////////////

namespace triagens {
  namespace mvcc {

     std::ostream& operator<< (std::ostream& stream,
                               Transaction const* transaction) {
       stream << transaction->toString();
       return stream;
     }
     
     std::ostream& operator<< (std::ostream& stream,
                               Transaction const& transaction) {
       stream << transaction.toString();
       return stream;
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
