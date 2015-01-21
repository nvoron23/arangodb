////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC master pointer manager
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

#ifndef ARANGODB_MVCC_MASTER_POINTER_MANAGER_H
#define ARANGODB_MVCC_MASTER_POINTER_MANAGER_H 1

#include "Basics/Common.h"
#include "Basics/Mutex.h"

struct TRI_doc_mptr_t;

namespace triagens {
  namespace mvcc {

    class MasterpointerManager;

// -----------------------------------------------------------------------------
// --SECTION--                                     struct MasterpointerContainer
// -----------------------------------------------------------------------------

    struct MasterpointerContainer {
      public:
        
        MasterpointerContainer (MasterpointerContainer const&) = delete;
        MasterpointerContainer& operator= (MasterpointerContainer const&) = delete;
        
        MasterpointerContainer (MasterpointerContainer&&);

        MasterpointerContainer (MasterpointerManager*,
                                struct TRI_doc_mptr_t*);

        ~MasterpointerContainer ();

        void setDataPtr (void const*);
        TRI_doc_mptr_t* operator* () const;

        MasterpointerManager* const manager;
        struct TRI_doc_mptr_t*      mptr;

    };

// -----------------------------------------------------------------------------
// --SECTION--                                        class MasterpointerManager
// -----------------------------------------------------------------------------

    class MasterpointerManager {

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

      public:

        MasterpointerManager (MasterpointerManager const&) = delete;
        MasterpointerManager& operator= (MasterpointerManager const&) = delete;

        MasterpointerManager ();

        ~MasterpointerManager ();

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new master pointer in the collection
/// the master pointer is not yet linked
////////////////////////////////////////////////////////////////////////////////

        MasterpointerContainer create ();

////////////////////////////////////////////////////////////////////////////////
/// @brief recycle a master pointer in the collection
/// the caller must guarantee that the master pointer will not be read or
/// modified by itself or any other threads
////////////////////////////////////////////////////////////////////////////////

        void recycle (TRI_doc_mptr_t*);

// -----------------------------------------------------------------------------
// --SECTION--                                                  private methods
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief determines the size of the block with position n
////////////////////////////////////////////////////////////////////////////////

        size_t getBlockSize (size_t) const;

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief a mutex protecting the _freelist and the _blocks
////////////////////////////////////////////////////////////////////////////////

        triagens::basics::Mutex      _lock;

////////////////////////////////////////////////////////////////////////////////
/// @brief current freelist head
////////////////////////////////////////////////////////////////////////////////

        TRI_doc_mptr_t*              _freelist;

////////////////////////////////////////////////////////////////////////////////
/// @brief blocks with allocated memory
////////////////////////////////////////////////////////////////////////////////

        std::vector<TRI_doc_mptr_t*> _blocks;
        
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
