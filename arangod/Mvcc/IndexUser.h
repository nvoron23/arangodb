////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC index user class
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

#ifndef ARANGODB_MVCC_INDEX_USER_H
#define ARANGODB_MVCC_INDEX_USER_H 1

#include "Basics/Common.h"
#include "Utils/Exception.h"

namespace triagens {
  namespace mvcc {

    class Index;
    class PrimaryIndex;
    class TransactionCollection;

// -----------------------------------------------------------------------------
// --SECTION--                                                   class IndexUser
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

    class IndexUser {

      public:

        IndexUser (IndexUser const&) = delete;
        IndexUser& operator= (IndexUser const&) = delete;

        IndexUser (TransactionCollection*);

        ~IndexUser ();

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief return a reference to the list of indexes
////////////////////////////////////////////////////////////////////////////////

        inline std::vector<triagens::mvcc::Index*> const& indexes () const {
          return _indexes;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief note that we need to click
////////////////////////////////////////////////////////////////////////////////

        inline void mustClick () {
          _mustClick = true;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not the collection has a cap constraint
////////////////////////////////////////////////////////////////////////////////

        bool hasCapConstraint () const;

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not the collection has secondary indexes
////////////////////////////////////////////////////////////////////////////////

        bool hasSecondaryIndexes () const;

////////////////////////////////////////////////////////////////////////////////
/// @brief return a reference to the primary index
////////////////////////////////////////////////////////////////////////////////

        triagens::mvcc::PrimaryIndex* primaryIndex () const;

////////////////////////////////////////////////////////////////////////////////
/// @brief "click" all indexes to enforce a synchronization
////////////////////////////////////////////////////////////////////////////////

        void click ();

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

      private:
       
        TransactionCollection*     _collection;
        std::vector<Index*>        _indexes;
        bool                       _mustClick;

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
