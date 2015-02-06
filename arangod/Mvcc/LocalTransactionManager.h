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
#include "Basics/Thread.h"
#include "Mvcc/Transaction.h"
#include "Mvcc/TransactionId.h"
#include "Mvcc/TransactionManager.h"

struct TRI_vocbase_s;

namespace triagens {
  namespace mvcc {

    class LocalTransactionManager;
    class TopLevelTransaction;

// -----------------------------------------------------------------------------
// --SECTION--                              LocalTransactionManagerCleanupThread
// -----------------------------------------------------------------------------

    class LocalTransactionManagerCleanupThread : public basics::Thread {

      public:

        LocalTransactionManagerCleanupThread (LocalTransactionManager*,
                                              double);
        
        ~LocalTransactionManagerCleanupThread ();

        bool init ();
      
        void stop ();

      protected:

        void run ();

      private:

        LocalTransactionManager* const _manager;
        
        double _defaultTtl;

        std::atomic<bool> _stopped;
    };


// -----------------------------------------------------------------------------
// --SECTION--                                     class LocalTransactionManager
// -----------------------------------------------------------------------------

    class LocalTransactionManager final : public TransactionManager {

      friend LocalTransactionManagerCleanupThread;

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
/// @brief create a top-level transaction and lease it
////////////////////////////////////////////////////////////////////////////////

        Transaction* createTransaction (struct TRI_vocbase_s*,
                                        double) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief create a sub-transaction and lease it
////////////////////////////////////////////////////////////////////////////////

        Transaction* createSubTransaction (struct TRI_vocbase_s*,
                                           Transaction*,
                                           double) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief lease a transaction
////////////////////////////////////////////////////////////////////////////////

        Transaction* leaseTransaction (TransactionId::InternalType) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief unlease a transaction
////////////////////////////////////////////////////////////////////////////////

        void unleaseTransaction (Transaction*) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief commit a transaction
////////////////////////////////////////////////////////////////////////////////

        void commitTransaction (TransactionId::InternalType) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief roll back a transaction
////////////////////////////////////////////////////////////////////////////////

        void rollbackTransaction (TransactionId::InternalType) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief kill a transaction
////////////////////////////////////////////////////////////////////////////////

        void killTransaction (TransactionId::InternalType) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief get a list of all running transactions
////////////////////////////////////////////////////////////////////////////////

        std::vector<TransactionInfo> runningTransactions (struct TRI_vocbase_s*) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the status of a transaction
////////////////////////////////////////////////////////////////////////////////

        Transaction::StatusType statusTransaction (TransactionId const&) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief adds the transactions from the parameter to the list of failed
/// transactions
////////////////////////////////////////////////////////////////////////////////

        void addFailedTransactions (std::unordered_set<TransactionId::InternalType> const&) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief removes the transaction from the list of running transactions and
/// deletes the transaction object
////////////////////////////////////////////////////////////////////////////////

        void deleteRunningTransaction (Transaction*,
                                       bool) override final;

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief initializes a transaction with state
////////////////////////////////////////////////////////////////////////////////

        void initializeTransaction (Transaction*) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief increment the transaction id counter and return it
////////////////////////////////////////////////////////////////////////////////

        TransactionId nextId ();

////////////////////////////////////////////////////////////////////////////////
/// @brief insert the transaction into the list of running transactions
////////////////////////////////////////////////////////////////////////////////

        void insertRunningTransaction (Transaction*);

////////////////////////////////////////////////////////////////////////////////
/// @brief kill long-running transactions if they are older than maxAge
/// seconds. this only sets the transactions' kill bit, but does not physically
/// remove them
////////////////////////////////////////////////////////////////////////////////

        bool killRunningTransactions (double);

////////////////////////////////////////////////////////////////////////////////
/// @brief physically remove killed transactions
////////////////////////////////////////////////////////////////////////////////
        
        void deleteKilledTransactions ();

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

        std::unordered_map<TransactionId::InternalType, Transaction*> _runningTransactions;

////////////////////////////////////////////////////////////////////////////////
/// @brief list of all failed transactions
////////////////////////////////////////////////////////////////////////////////

        std::unordered_set<TransactionId::InternalType> _failedTransactions;

////////////////////////////////////////////////////////////////////////////////
/// @brief list of all currently leased transactions
/// a least transaction is exclusively given to a specific thread and must not
/// be modified by another
////////////////////////////////////////////////////////////////////////////////

        std::unordered_set<TransactionId::InternalType> _leasedTransactions;

////////////////////////////////////////////////////////////////////////////////
/// @brief a cleanup thread for killing and deleting abandoned transactions
////////////////////////////////////////////////////////////////////////////////

        LocalTransactionManagerCleanupThread* _cleanupThread;

////////////////////////////////////////////////////////////////////////////////
/// @brief default transaction expiration time
////////////////////////////////////////////////////////////////////////////////

        static double const DefaultTtl;
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
