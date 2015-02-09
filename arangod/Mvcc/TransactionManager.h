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

#ifndef ARANGODB_MVCC_TRANSACTION_MANAGER_H
#define ARANGODB_MVCC_TRANSACTION_MANAGER_H 1

#include "Basics/Common.h"
#include "Mvcc/Transaction.h"
#include "Mvcc/TransactionId.h"

struct TRI_vocbase_s;

namespace triagens {
  namespace mvcc {

    class TopLevelTransaction;

// -----------------------------------------------------------------------------
// --SECTION--                                          class TransactionManager
// -----------------------------------------------------------------------------

    class TransactionManager {
      
// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

      public:
      
        TransactionManager (TransactionManager const&) = delete;
        TransactionManager& operator= (TransactionManager const&) = delete;

////////////////////////////////////////////////////////////////////////////////
/// @brief create the transaction manager
////////////////////////////////////////////////////////////////////////////////

        TransactionManager ();

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the transaction manager
////////////////////////////////////////////////////////////////////////////////

        virtual ~TransactionManager ();

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief create a top-level transaction
////////////////////////////////////////////////////////////////////////////////

        virtual Transaction* createTransaction (struct TRI_vocbase_s*,
                                                std::map<std::string, bool> const& collections,
                                                double ttl) = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief create a sub-transaction
////////////////////////////////////////////////////////////////////////////////

        virtual Transaction* createSubTransaction (struct TRI_vocbase_s*, 
                                                   Transaction*,
                                                   std::map<std::string, bool> const& collections,
                                                   double ttl) = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief lease a transaction
////////////////////////////////////////////////////////////////////////////////

        virtual Transaction* leaseTransaction (TransactionId::InternalType) = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief unlease a transaction
////////////////////////////////////////////////////////////////////////////////

        virtual void unleaseTransaction (Transaction*) = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief commit a transaction by id
////////////////////////////////////////////////////////////////////////////////

        virtual void commitTransaction (TransactionId::InternalType) = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief roll back a transaction by id
////////////////////////////////////////////////////////////////////////////////

        virtual void rollbackTransaction (TransactionId::InternalType) = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief kill a transaction
////////////////////////////////////////////////////////////////////////////////

        virtual void killTransaction (TransactionId::InternalType) = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief get a list of all running transactions
////////////////////////////////////////////////////////////////////////////////

        virtual std::vector<TransactionInfo> runningTransactions (struct TRI_vocbase_s*) = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief initializes a transaction with state
////////////////////////////////////////////////////////////////////////////////

        virtual void initializeTransaction (Transaction*) = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the status of a transaction
////////////////////////////////////////////////////////////////////////////////

        virtual Transaction::StatusType statusTransaction (TransactionId const&) = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief adds the transactions from the parameter to the list of failed
/// transactions
////////////////////////////////////////////////////////////////////////////////

        virtual void addFailedTransactions (std::unordered_set<TransactionId::InternalType> const&) = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief remove the transaction from the list of running transactions
////////////////////////////////////////////////////////////////////////////////

        virtual void deleteRunningTransaction (Transaction*) = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief initialize the transaction manager
////////////////////////////////////////////////////////////////////////////////

        static void initialize ();

////////////////////////////////////////////////////////////////////////////////
/// @brief shutdown the transaction manager
////////////////////////////////////////////////////////////////////////////////

        static void shutdown ();

////////////////////////////////////////////////////////////////////////////////
/// @brief return the global transaction manager instance
////////////////////////////////////////////////////////////////////////////////

        static TransactionManager* instance ();

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief the global transaction manager instance
////////////////////////////////////////////////////////////////////////////////

        static TransactionManager* Instance;
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
