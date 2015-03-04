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

#ifndef ARANGODB_MVCC_SUB_TRANSACTION_H
#define ARANGODB_MVCC_SUB_TRANSACTION_H 1

#include "Basics/Common.h"
#include "Mvcc/CollectionStats.h"
#include "Mvcc/Transaction.h"

namespace triagens {
  namespace arango {
    class CollectionNameResolver;
  }

  namespace mvcc {

    class TopLevelTransaction;
    class TransactionCollection;

// -----------------------------------------------------------------------------
// --SECTION--                                              class SubTransaction
// -----------------------------------------------------------------------------

    class SubTransaction final : public Transaction {

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new sub transaction
////////////////////////////////////////////////////////////////////////////////

        SubTransaction (Transaction*,
                        std::map<std::string, bool> const&,
                        double);

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the transaction
////////////////////////////////////////////////////////////////////////////////

        ~SubTransaction ();

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief commit the transaction
////////////////////////////////////////////////////////////////////////////////
   
        void commit () override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief roll back the transaction
////////////////////////////////////////////////////////////////////////////////
   
        void rollback () override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief returns aggregated transaction statistics
////////////////////////////////////////////////////////////////////////////////

        CollectionStats aggregatedStats (TRI_voc_cid_t) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief returns a collection used in the transaction
/// this registers the collection in the transaction if not yet present
////////////////////////////////////////////////////////////////////////////////
        
        TransactionCollection* collection (std::string const&) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief returns a collection used in the transaction
/// this registers the collection in the transaction if not yet present
////////////////////////////////////////////////////////////////////////////////
        
        TransactionCollection* collection (char const*) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief returns a collection used in the transaction
/// this registers the collection in the transaction if not yet present
////////////////////////////////////////////////////////////////////////////////
        
        TransactionCollection* collection (TRI_voc_cid_t) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief return a collection name resolver instance
////////////////////////////////////////////////////////////////////////////////

        triagens::arango::CollectionNameResolver const* resolver () override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief register an external collection name resolver instance
////////////////////////////////////////////////////////////////////////////////
        
        void resolver (triagens::arango::CollectionNameResolver const*) override final;

////////////////////////////////////////////////////////////////////////////////
/// @brief get the transaction's top-level transaction
////////////////////////////////////////////////////////////////////////////////

        TopLevelTransaction* topLevelTransaction () override final {
          return _topLevelTransaction;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief get the transaction's top-level transaction
////////////////////////////////////////////////////////////////////////////////

        TopLevelTransaction const* topLevelTransaction () const override final {
          return _topLevelTransaction;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief get the transaction's direct parent transaction
////////////////////////////////////////////////////////////////////////////////

        Transaction* parentTransaction () override final {
          return _parentTransaction;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not the transaction is a top-level transaction
////////////////////////////////////////////////////////////////////////////////

        bool isTopLevel () const override final {
          return false;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief prepare the statistics so updating them later is guaranteed to
/// succeed
////////////////////////////////////////////////////////////////////////////////

        void prepareStats (TransactionCollection const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief get a string representation of the transaction
////////////////////////////////////////////////////////////////////////////////

        std::string toString () const override final;

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief this transaction's top-level transaction
////////////////////////////////////////////////////////////////////////////////

        TopLevelTransaction* const _topLevelTransaction;

////////////////////////////////////////////////////////////////////////////////
/// @brief this transaction's direct parent transaction
////////////////////////////////////////////////////////////////////////////////

        Transaction* _parentTransaction;

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
