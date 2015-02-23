////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC collection read locker
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

#ifndef ARANGODB_MVCC_COLLECTION_READ_LOCK_H
#define ARANGODB_MVCC_COLLECTION_READ_LOCK_H 1

#include "Basics/Common.h"

namespace triagens {
  namespace mvcc {

    class Transaction;
    class TransactionCollection;

// -----------------------------------------------------------------------------
// --SECTION--                                          class CollectionReadLock
// -----------------------------------------------------------------------------

    class CollectionReadLock {

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

      public:

        CollectionReadLock (CollectionReadLock const&) = delete;
        CollectionReadLock& operator= (CollectionReadLock const&) = delete;

        CollectionReadLock (TransactionCollection*,
                            Transaction*);

        ~CollectionReadLock ();

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief the underlying collection
////////////////////////////////////////////////////////////////////////////////

        TransactionCollection* _collection;

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not we have locked the collection
////////////////////////////////////////////////////////////////////////////////

        bool _hasLocked;

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
