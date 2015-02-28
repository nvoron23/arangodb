////////////////////////////////////////////////////////////////////////////////
/// @brief fast block allocator
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2015 ArangoDB GmbH, Cologne, Germany
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
/// @author Max Neunhoeffer
/// @author Copyright 2015, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGODB_BASICS_ALLOCATOR_H
#define ARANGODB_BASICS_ALLOCATOR_H 1

#include "Basics/Common.h"
#include "Basics/Mutex.h"
#include "Basics/MutexLocker.h"

namespace triagens {
  namespace basics {

    // Fundamental ideas:
    //
    // We use a class "Allocator" to allocate lots of blocks of the
    // same size, which must be divisible by 4 and not more than
    // about 1024 bytes. The blocks of the same size are allocated in
    // "Bricks", where a brick is always 256kB and is fully aligned to
    // 256kB boundaries. Therefore one can quickly read off from a void*
    // pointing to a block, to which Brick it belongs.
    // To achieve this alignment we always allocate Bricks in "Chunks".
    // A Chunk contains 512 Bricks plus some additional information in
    // the header, totalling about 128MB. There will be a single malloc
    // for each Chunk. We achieve 256kB alignment by allocating 256kB
    // more, which is very little in comparion to 128MB.
    // Each Brick remembers a pointer to its Chunk. Note that the block
    // size in each Brick can be different, even within the same Chunk.
    // The data structure to administrate all the Chunks we have
    // allocated is called "Arena", we will probably only need a single
    // Arena across the whole application.
    // The Allocator class depends on an Arena to get Bricks, all of
    // which will have the same block size.

    // Ultimately, we want allocation and freeing of blocks to be fast,
    // the first measure to achieve this is that we use few mallocs.
    // Within a Brick, all blocks have the same size and therefore we
    // can usually hand out a new one by only increasing one counter.
    // Additionally we administrate a free list (singly linked list) to
    // allow for freeing and later reuse. We want to avoid locking or
    // mutexes for the allocation as much as possible. This is done as
    // follows: In the Allocator, each thread has a thread_local Brick,
    // from which it allocates blocks. There is a Mutex for a thread to
    // acquire a current Brick. Since no other thread is allocating from
    // this Brick, we can do this nearly without locking. The problem is
    // the freeing, which can happen from any thread concurrently. We need
    // to protect the single allocating thread taking stuff off the free
    // list from the possibly multiple threads putting stuff on the free
    // list. We use int32_t values to specify the index of a block in
    // a brick. The free list is a single such number, the value -1 stands
    // for an empty free list and otherwise it contains the index of the
    // first member of the free list. The first 4 bytes of a freed block
    // contain the next member of the linked free list, the last member
    // has -1 there.

    // Here is how we add index i to the free list:
    //   while (true) {
    //     load freeList value to v (std::memory_order_relaxed)
    //     write v to the beginning of the block to be freed
    //     use compare-and-swap with memory order std::memory_order_release
    //         to change freeList from v to i
    //       break if this has worked
    //   }

    // Here is how we remove the first entry of the free list:
    // (note that there is always only one thread at the same time doing this!)
    //   while (true) {
    //     load freeList value to v (memory order acquire)
    //     if v is -1 then give up and report no block in here
    //     find corresponding block and read next value there
    //     use compare-and-swap with memory order std::memory_order_release
    //         to change freeList from v to next
    //       break if this has worked
    //   }

    // forward declarations:
    class Brick;
    class Chunk;

    class Arena {

      friend class Allocator;

        // a fully thread-safe memory management for Bricks, they are allocated
        // in Chunks of 512 Bricks (which is 128MB)
        Mutex _mutex;
        std::unordered_set<Chunk*> _fullChunks;
        std::unordered_set<Chunk*> _notFullChunks;
        size_t _bricksPerChunk;

        Arena (size_t bricksPerChunk) 
          : _bricksPerChunk(bricksPerChunk) {
        }

        ~Arena ();

      private:

        Brick* leaseBrick (size_t blockSize);

        void returnBrick (Brick* brick);
    };

    class Allocator {
        // This structure is allocated on the heap as usual and administrates
        // arbitrarily many Small structures in a thread local way. It gives
        // every thread a "current" Small structure and organises the locking
        // it draws the allocations
        //
        // Thread local _current, plus protected _full???
        Mutex _mutex;
        Arena* _arena;
        size_t _blockSize;
        static thread_local Brick* _current;
        std::unordered_set<Brick*> _isFull;
        std::unordered_set<Brick*> _notFull;
        std::unordered_set<Brick*> _isCurrent;

      public:

        Allocator (Arena* arena, size_t blockSize)
          : _arena(arena), _blockSize(blockSize) {
        }

        ~Allocator ();

        void* allocate ();

        void free (void*);
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
