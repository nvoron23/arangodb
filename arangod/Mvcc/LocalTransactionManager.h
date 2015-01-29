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

#ifndef ARANGODB_MVCC_LOCAL_TRANSACTION_MANAGER_H
#define ARANGODB_MVCC_LOCAL_TRANSACTION_MANAGER_H 1

#include "Basics/Common.h"
#include "Basics/ReadWriteLock.h"
#include "Mvcc/Transaction.h"
#include "Mvcc/TransactionId.h"
#include "Mvcc/TransactionManager.h"

struct TRI_vocbase_s;

namespace triagens {
  namespace mvcc {

    class TopLevelTransaction;

// -----------------------------------------------------------------------------
// --SECTION--                                     class LocalTransactionManager
// -----------------------------------------------------------------------------

    class LocalTransactionManager final : public TransactionManager {

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief create the transaction manager
////////////////////////////////////////////////////////////////////////////////

        LocalTransactionManager ();

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the transaction manager
////////////////////////////////////////////////////////////////////////////////

        ~LocalTransactionManager ();

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief create a top-level transaction
////////////////////////////////////////////////////////////////////////////////

        Transaction* createTransaction (struct TRI_vocbase_s*) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief create a sub-transaction
////////////////////////////////////////////////////////////////////////////////

        Transaction* createSubTransaction (struct TRI_vocbase_s*,
                                           Transaction*) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief unregister a transaction
////////////////////////////////////////////////////////////////////////////////

        void unregisterTransaction (Transaction*) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief commit a transaction
////////////////////////////////////////////////////////////////////////////////

        void commitTransaction (TransactionId::IdType) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief roll back a transaction
////////////////////////////////////////////////////////////////////////////////

        void rollbackTransaction (TransactionId::IdType) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief abort a transaction
////////////////////////////////////////////////////////////////////////////////

        void abortTransaction (TransactionId::IdType) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief get a list of all running transactions
////////////////////////////////////////////////////////////////////////////////

        std::vector<TransactionInfo> runningTransactions (struct TRI_vocbase_s*) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief initializes a transaction with state
////////////////////////////////////////////////////////////////////////////////

        void initializeTransaction (Transaction*) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the status of a transaction
////////////////////////////////////////////////////////////////////////////////

        Transaction::StatusType statusTransaction (TransactionId::IdType) override final;

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief increment the transaction id counter and return it
////////////////////////////////////////////////////////////////////////////////

        TransactionId nextId ();

////////////////////////////////////////////////////////////////////////////////
/// @brief insert the transaction into the list of running transactions
////////////////////////////////////////////////////////////////////////////////

        void insertRunningTransaction (Transaction*);

////////////////////////////////////////////////////////////////////////////////
/// @brief remove the transaction from the list of running transactions
////////////////////////////////////////////////////////////////////////////////

        void removeRunningTransaction (Transaction*);

////////////////////////////////////////////////////////////////////////////////
/// @brief push the transaction onto the thread-local stack
////////////////////////////////////////////////////////////////////////////////

        void pushOnThreadStack (Transaction*);

////////////////////////////////////////////////////////////////////////////////
/// @brief pop the transaction from the thread-local stack
////////////////////////////////////////////////////////////////////////////////

        void popFromThreadStack (Transaction*);

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief lock for _runningTransactions
////////////////////////////////////////////////////////////////////////////////

        triagens::basics::ReadWriteLock _lock;

////////////////////////////////////////////////////////////////////////////////
/// @brief list of all currently running transactions
////////////////////////////////////////////////////////////////////////////////

        std::unordered_map<TransactionId::IdType, Transaction*> _runningTransactions;

////////////////////////////////////////////////////////////////////////////////
/// @brief list of all failed transactions
////////////////////////////////////////////////////////////////////////////////

        std::unordered_set<TransactionId::IdType> _failedTransactions;

////////////////////////////////////////////////////////////////////////////////
/// @brief thread-local vector of started top-level transactions
////////////////////////////////////////////////////////////////////////////////
    
        static thread_local std::vector<Transaction*> _threadTransactions;

    };

  }
}

#endif

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
