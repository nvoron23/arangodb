////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC transaction scope
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

#ifndef ARANGODB_MVCC_TRANSACTION_SCOPE_H
#define ARANGODB_MVCC_TRANSACTION_SCOPE_H 1

#include "Basics/Common.h"
#include "Mvcc/Transaction.h"

struct TRI_vocbase_s;

namespace triagens {
  namespace mvcc {

    class Transaction;
    class TransactionManager;

// -----------------------------------------------------------------------------
// --SECTION--                                            class TransactionScope
// -----------------------------------------------------------------------------

    class TransactionScope {

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

      public:

        TransactionScope (TransactionScope const&) = delete;
        TransactionScope& operator= (TransactionScope const&) = delete;
      
////////////////////////////////////////////////////////////////////////////////
/// @brief join an existing transaction in an outer scope or a new transaction, 
/// which will be automatically freed when the scope is left
////////////////////////////////////////////////////////////////////////////////

        TransactionScope (struct TRI_vocbase_s*, 
                          std::map<std::string, bool> const& collections,
                          bool forceNew = false,
                          bool canBeSubTransaction = true,
                          double ttl = 0.0);

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the transaction scope
////////////////////////////////////////////////////////////////////////////////

        ~TransactionScope ();

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief returns an empty list of collections
////////////////////////////////////////////////////////////////////////////////

        static std::map<std::string, bool> const& NoCollections ();

////////////////////////////////////////////////////////////////////////////////
/// @brief whether we own the transaction
////////////////////////////////////////////////////////////////////////////////

        inline bool hasStartedTransaction () const {
          return _id > 0;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief return the transaction manager
////////////////////////////////////////////////////////////////////////////////

        inline TransactionManager* transactionManager () const {
          return _transactionManager;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief return the transaction
////////////////////////////////////////////////////////////////////////////////

        inline Transaction* transaction () const {
          TRI_ASSERT_EXPENSIVE(_transaction != nullptr);
          return _transaction;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief set the commit flag for the scoped transaction
////////////////////////////////////////////////////////////////////////////////

        void commit ();

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief remove the transaction from the stack
////////////////////////////////////////////////////////////////////////////////

        void removeFromStack ();

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief the transaction manager
////////////////////////////////////////////////////////////////////////////////

        TransactionManager* _transactionManager;

////////////////////////////////////////////////////////////////////////////////
/// @brief the transaction 
////////////////////////////////////////////////////////////////////////////////

        Transaction* _transaction;

////////////////////////////////////////////////////////////////////////////////
/// @brief transaction id 
/// if this is > 0, then we own the transaction!
////////////////////////////////////////////////////////////////////////////////

        TransactionId::InternalType _id;

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not the transaction was pushed onto the thread stack
////////////////////////////////////////////////////////////////////////////////

        bool _pushedOnThreadStack;

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not the transaction should commit
////////////////////////////////////////////////////////////////////////////////

        bool _shouldCommit;

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
