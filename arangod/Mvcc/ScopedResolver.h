////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC collection name resolver guard
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

#ifndef ARANGODB_MVCC_SCOPED_RESOLVER_H
#define ARANGODB_MVCC_SCOPED_RESOLVER_H 1

#include "Basics/Common.h"

struct TRI_vocbase_s;

namespace triagens {
  namespace arango {
    class CollectionNameResolver;
  }

  namespace mvcc {

// -----------------------------------------------------------------------------
// --SECTION--                                              class ScopedResolver
// -----------------------------------------------------------------------------

    class ScopedResolver {

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

      public:

        ScopedResolver (ScopedResolver const&) = delete;
        ScopedResolver& operator= (ScopedResolver const&) = delete;

        explicit ScopedResolver (struct TRI_vocbase_s*);

        ~ScopedResolver ();

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------
      
      public:

        triagens::arango::CollectionNameResolver const* get () const {
          return _resolver;
        }

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

      private:

        triagens::arango::CollectionNameResolver const* _resolver;
        bool _ownsResolver;
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
