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

struct TRI_document_collection_t;

namespace triagens {
  namespace mvcc {

    class Index;

// -----------------------------------------------------------------------------
// --SECTION--                                                   class IndexUser
// -----------------------------------------------------------------------------

    class IndexUser {

      public:

        IndexUser (IndexUser const&) = delete;
        IndexUser& operator= (IndexUser const&) = delete;

        IndexUser (struct TRI_document_collection_t*);

        ~IndexUser ();

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief return a reference to the list of indexes
////////////////////////////////////////////////////////////////////////////////

        inline std::vector<triagens::mvcc::Index*> const& indexes () const {
          return _indexes;
        }

      private:
       
        TRI_document_collection_t* _collection; 
        std::vector<Index*>        _indexes;

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
