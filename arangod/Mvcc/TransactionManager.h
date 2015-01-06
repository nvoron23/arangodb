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

struct TRI_vocbase_s;

namespace triagens {
  namespace mvcc {

    class TopLevelTransaction;
    class Transaction;

// -----------------------------------------------------------------------------
// --SECTION--                                          class TransactionManager
// -----------------------------------------------------------------------------

    class TransactionManager {

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

      public:

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

        virtual TopLevelTransaction* createTopLevelTransaction (struct TRI_vocbase_s*) = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief create a (potentially nested) transaction
////////////////////////////////////////////////////////////////////////////////

        virtual Transaction* createTransaction (struct TRI_vocbase_s*) = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief unregister a transaction
////////////////////////////////////////////////////////////////////////////////

        virtual void unregisterTransaction (Transaction const*) = 0;

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
