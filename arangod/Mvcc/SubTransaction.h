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
#include "Mvcc/Transaction.h"

namespace triagens {
  namespace mvcc {

    class TopLevelTransaction;

// -----------------------------------------------------------------------------
// --SECTION--                                              class SubTransaction
// -----------------------------------------------------------------------------

    class SubTransaction final : public Transaction {

      public:

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new sub transaction
////////////////////////////////////////////////////////////////////////////////

        SubTransaction (Transaction*);

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the transaction
////////////////////////////////////////////////////////////////////////////////

        ~SubTransaction ();

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief get the transaction's parent transaction
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
/// @brief get a string representation of the transaction
////////////////////////////////////////////////////////////////////////////////

        std::string toString () const override final;

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief this transaction's parent transaction
////////////////////////////////////////////////////////////////////////////////

        Transaction* const _parentTransaction;

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
