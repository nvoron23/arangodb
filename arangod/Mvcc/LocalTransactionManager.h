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
#include "Mvcc/Transaction.h"
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

        LocalTransactionManager (Transaction::IdType);

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

        TopLevelTransaction* createTopLevelTransaction (struct TRI_vocbase_s*) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief create a (potentially nested) transaction
////////////////////////////////////////////////////////////////////////////////

        Transaction* createTransaction (struct TRI_vocbase_s*) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief unregister a transaction
////////////////////////////////////////////////////////////////////////////////

        void unregisterTransaction (Transaction*) override final;

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief increment the id counter and return the new id
////////////////////////////////////////////////////////////////////////////////

        Transaction::IdType nextId ();

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief last handed-out transaction id
////////////////////////////////////////////////////////////////////////////////

        std::atomic<Transaction::IdType> _lastUsedId;

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
