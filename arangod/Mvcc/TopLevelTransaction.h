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

#ifndef ARANGODB_MVCC_TOP_LEVEL_TRANSACTION_H
#define ARANGODB_MVCC_TOP_LEVEL_TRANSACTION_H 1

#include "Basics/Common.h"
#include "Mvcc/Transaction.h"

struct TRI_vocbase_s;

namespace triagens {
  namespace mvcc {

    class SubTransaction;
    class TransactionManager;

// -----------------------------------------------------------------------------
// --SECTION--                                         class TopLevelTransaction
// -----------------------------------------------------------------------------

    class TopLevelTransaction final : public Transaction {

      friend class TransactionManager;
      friend class LocalTransactionManager;

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new top-level transaction
////////////////////////////////////////////////////////////////////////////////

        TopLevelTransaction (TransactionManager*,
                             TransactionId const&,
                             struct TRI_vocbase_s*);

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the transaction
////////////////////////////////////////////////////////////////////////////////

        ~TopLevelTransaction ();

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief get the transaction's top-level transaction (i.e. ourselves)
////////////////////////////////////////////////////////////////////////////////

        TopLevelTransaction* topLevelTransaction () override final {
          return this;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief get the transaction's direct parent transaction (i.e. ourselves)
////////////////////////////////////////////////////////////////////////////////

        Transaction* parentTransaction () override final {
          return this;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not the transaction is a top-level transaction
////////////////////////////////////////////////////////////////////////////////

        bool isTopLevel () const override final {
          return true;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief get a string representation of the transaction
////////////////////////////////////////////////////////////////////////////////

        std::string toString () const override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief return the next sub-transaction id
////////////////////////////////////////////////////////////////////////////////

        TransactionId nextSubId ();

// -----------------------------------------------------------------------------
// --SECTION--                                                 protected methods
// -----------------------------------------------------------------------------

      protected:

////////////////////////////////////////////////////////////////////////////////
/// @brief sets the start state (e.g. list of running transactions
////////////////////////////////////////////////////////////////////////////////

        void setStartState (std::unordered_set<TransactionId> const&);

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief transactions active at start
////////////////////////////////////////////////////////////////////////////////

        std::unordered_set<TransactionId>* _runningTransactions;

////////////////////////////////////////////////////////////////////////////////
/// @brief last assigned sub-transaction id
////////////////////////////////////////////////////////////////////////////////

        TransactionId::IdType _lastUsedSubId;

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
