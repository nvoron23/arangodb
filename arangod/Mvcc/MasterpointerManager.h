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
#include "Mvcc/Transaction.h"
#include "Mvcc/TransactionId.h"
#include "VocBase/document-collection.h"

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

        TRI_doc_mptr_t const* operator* () const {
          return mptr;
        }
        
        TRI_doc_mptr_t* operator* () {
          return mptr;
        }

        TRI_doc_mptr_t* operator-> () {
          return mptr;
        }

        TRI_doc_mptr_t const* get () const {
          return mptr;
        }
        
        TRI_doc_mptr_t* get () {
          return mptr;
        }

        void link ();

        MasterpointerManager* const manager;
        struct TRI_doc_mptr_t*      mptr;
        bool                        owns;

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

        MasterpointerContainer create (void const*,
                                       triagens::mvcc::TransactionId const&);

////////////////////////////////////////////////////////////////////////////////
/// @brief link the master pointer
////////////////////////////////////////////////////////////////////////////////

        void link (TRI_doc_mptr_t*);

////////////////////////////////////////////////////////////////////////////////
/// @brief unlink the master pointer from the list
////////////////////////////////////////////////////////////////////////////////

        void unlink (TRI_doc_mptr_t*);

////////////////////////////////////////////////////////////////////////////////
/// @brief recycle a master pointer in the collection
/// the caller must guarantee that the master pointer will not be read or
/// modified by itself or any other threads
////////////////////////////////////////////////////////////////////////////////

        void recycle (TRI_doc_mptr_t*);

////////////////////////////////////////////////////////////////////////////////
/// @brief initialize an iterator with the current list head and tails
////////////////////////////////////////////////////////////////////////////////

        void initializeIterator (TRI_doc_mptr_t const*& head, 
                                 TRI_doc_mptr_t const*& tail);

////////////////////////////////////////////////////////////////////////////////
/// @brief to be called when an iterator is finished
////////////////////////////////////////////////////////////////////////////////

        void shutdownIterator ();

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
/// @brief head of linked, publicly visible master pointers list
////////////////////////////////////////////////////////////////////////////////

        TRI_doc_mptr_t*              _head;

////////////////////////////////////////////////////////////////////////////////
/// @brief tail of linked, publicly visible master pointers list
////////////////////////////////////////////////////////////////////////////////

        TRI_doc_mptr_t*              _tail;

////////////////////////////////////////////////////////////////////////////////
/// @brief blocks with allocated memory
////////////////////////////////////////////////////////////////////////////////

        std::vector<TRI_doc_mptr_t*> _blocks;

////////////////////////////////////////////////////////////////////////////////
/// @brief list of master pointers to recycle later
////////////////////////////////////////////////////////////////////////////////

        std::vector<TRI_doc_mptr_t*> _toRecycle;

////////////////////////////////////////////////////////////////////////////////
/// @brief number of iterators currently active
////////////////////////////////////////////////////////////////////////////////

        std::atomic<int64_t>         _numIteratorsActive;
        
    };

// -----------------------------------------------------------------------------
// --SECTION--                                      struct MasterpointerIterator
// -----------------------------------------------------------------------------

    struct MasterpointerIterator {
      MasterpointerIterator (Transaction* transaction,
                             MasterpointerManager* manager,
                             bool reverse)
        : transaction(transaction),
          manager(manager),
          current(nullptr),
          head(nullptr),
          tail(nullptr),
          reverse(reverse) {

        manager->initializeIterator(head, tail);

        if (reverse) {
          current = tail;
        }
        else {
          current = head;
        }
      }

      ~MasterpointerIterator () {
        manager->shutdownIterator();
      }

      bool hasMore () const {
        return (current != nullptr);
      }

      void next (std::vector<TRI_doc_mptr_t const*>& result,
                 int64_t& skip,
                 int64_t& limit,
                 int64_t batchSize) {
        TRI_ASSERT(limit > 0);

        while (current != nullptr && limit > 0 && batchSize > 0) {
          bool isVisible = transaction->isVisibleForRead(current->from(), current->to());

          if (isVisible) {
            if (skip > 0) {
              --skip;
            }
            else {
              result.push_back(current);
              --limit;
              --batchSize;
            }
          }

          if (reverse) {
            // reverse iterator
            if (current == head) {
              current = nullptr;
              break;
            }

            // previous element
            current = current->_prev;
          }
          else {
            // forward iterator
            if (current == tail) {
              current = nullptr;
              break;
            }

            // next element
            current = current->_next;
          }
        }

        if (limit == 0) {
          current = nullptr;
        }
      }
      
      TRI_doc_mptr_t const* next (int64_t& skip,
                                  int64_t& limit) {
        TRI_ASSERT(limit > 0);
        TRI_doc_mptr_t const* result = nullptr;

        while (result == nullptr && current != nullptr && limit > 0) {
          bool isVisible = transaction->isVisibleForRead(current->from(), current->to());

          if (isVisible) {
            if (skip > 0) {
              --skip;
            }
            else {
              result = current;
              --limit;
            }
          }

          if (reverse) {
            // reverse iterator
            if (current == head) {
              current = nullptr;
              break;
            }

            // previous element
            current = current->_prev;
          }
          else {
            // forward iterator
            if (current == tail) {
              current = nullptr;
              break;
            }

            // next element
            current = current->_next;
          }
        }

        if (limit == 0) {
          current = nullptr;
        }

        return result;
      }
      
      TRI_doc_mptr_t const* next () {
        TRI_doc_mptr_t const* result = nullptr;

        while (result == nullptr && current != nullptr) {
          bool isVisible = transaction->isVisibleForRead(current->from(), current->to());

          if (isVisible) {
            result = current;
          }

          if (reverse) {
            // reverse iterator
            if (current == head) {
              current = nullptr;
              break;
            }

            // previous element
            current = current->_prev;
          }
          else {
            // forward iterator
            if (current == tail) {
              current = nullptr;
              break;
            }

            // next element
            current = current->_next;
          }
        }

        return result;
      }

      Transaction* transaction;
      MasterpointerManager* const manager;
      TRI_doc_mptr_t const* current;
      TRI_doc_mptr_t const* head;
      TRI_doc_mptr_t const* tail;
      bool reverse;
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
