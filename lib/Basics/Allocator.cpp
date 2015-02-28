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

#include "Basics/Allocator.h"

using namespace triagens::basics;

// -----------------------------------------------------------------------------
// --SECTION--                                               class Brick, 256kB
// -----------------------------------------------------------------------------

class triagens::basics::Brick {
    // must use placement new with a 256kB aligned address as follows
    //   Brick* p = new(pos) Brick(blockSize, char* chunk)
    // and use explicit destruction as follows:
    //   p->~Brick()
    // is always 256kB large and fully aligned on 256kB boundaries
    // has a small header for administration and then administrates
    // (256*1024-64)/blockSize blocks of memory, where blockSize is
    // at least 4, this is a thread-local structure that does not 
    // perform locking at all. Alignment of blocks is always 
    // gcd(64, blockSize)
    char* _start;               // start of the blocks
    Chunk* const _chunk;        // base of chunk data structure
    size_t const _blockSize;    // size of a single block
    int32_t const _nrBlocks;    // this is always (256*1024-64)/allocSize
    int32_t _nextUnused;        // next unused block
    std::atomic<int32_t> _nrFree;      // number of unused blocks
    std::atomic<int32_t> _freeList;    // start of free list, -1 is empty
    char padding[64-(1 * sizeof(char*) +
                     1 * sizeof(Chunk*) +
                     1 * sizeof(size_t) +
                     2 * sizeof(int32_t) +
                     2 * sizeof(std::atomic<int32_t>) +
                     1 * sizeof(size_t) + sizeof(char*) + sizeof(Chunk*))];
                 // achieve sizeof(Brick)==64

  public:
    static size_t const totalSize = 256*1024;  // must be a 2-power
    // Here we use that numbers are stored as twos complement:
    static uintptr_t const mask =   static_cast<uintptr_t>(0)
                                  - static_cast<uintptr_t>(totalSize);

  private:
    static size_t const blocksSpace;

    void* block (int32_t pos) {
      return pos == -1 ? 
             nullptr : 
             static_cast<void*>(_start + 
                                pos * static_cast<int32_t>(_blockSize));
    }

    int32_t index (void* block) {
      return static_cast<int32_t>
             ((static_cast<char*>(block) - _start) / _blockSize);
    }

  public:

    Brick (size_t blockSize, Chunk* chunk)
      : _chunk(chunk),
        _blockSize(blockSize),
        _nrBlocks(static_cast<int32_t>(blocksSpace / blockSize)) {

      _nrFree.store(static_cast<int32_t>(blocksSpace / blockSize));
      _start = reinterpret_cast<char*>(this) + sizeof(Brick);
      _nextUnused = 0;
      _freeList.store(-1);
      TRI_ASSERT(_blockSize > 0 &&
                 (_blockSize & 3) == 0);  // Must be divisible by 4
    }

    ~Brick () {
    }
    
    Chunk* chunk () const {
      return _chunk;
    }

    static Brick* findBrick(void* block) {
      uintptr_t pos = reinterpret_cast<uintptr_t>(block);
      pos &= mask;
      return reinterpret_cast<Brick*>(pos);
    }

    void* allocate () {
      // Note that this runs exclusively in one thread for each brick!
      if (_nextUnused < _nrBlocks) {
        _nrFree--;
        return block(_nextUnused++);
      }

      while (true) {
        int32_t freeList = _freeList.load(std::memory_order_acquire);
        if (freeList == -1) {
          return nullptr;
        }
        void* res = block(freeList);
        int32_t* peek = static_cast<int32_t*>(res);
        int32_t next = *peek;
        if (_freeList.compare_exchange_weak(freeList, next, 
                       std::memory_order_release,
                       std::memory_order_relaxed)) {
          _nrFree--;
          return res;
        }
      }

    }

    static bool free (void* block) {
      Brick* brick = findBrick(block);
      int32_t pos = brick->index(block);
      int32_t* poke = reinterpret_cast<int32_t*>(block);
      while (true) {
        int32_t freeList = brick->_freeList.load(std::memory_order_acquire);

        *poke = freeList;
        if (brick->_freeList.compare_exchange_weak(freeList, pos, 
                                         std::memory_order_release,
                                         std::memory_order_relaxed)) {
          brick->_nrFree++;
          return brick->empty();
        }
      }
    }

    size_t nrBlocks () const {
      return _nrBlocks;
    }

    size_t nrUsed () const {
      return _nrBlocks - _nrFree;
    }

    size_t nrFree () const {
      return _nrFree.load();
    }

    bool empty () const {
      return _nrFree.load() == _nrBlocks;
    }

    bool full () const {
      return _nrFree.load() == 0;
    }
};

size_t const Brick::blocksSpace = Brick::totalSize - sizeof(Brick);

// -----------------------------------------------------------------------------
// --SECTION--                                               class Chunk, 128mB
// -----------------------------------------------------------------------------

class triagens::basics::Chunk {
    // must use placement new with a memory location pos at which at
    // least nrBricks*(256kB)+sizeof(Chunk) is allocated and such that
    // pos+sizeof(Chunk) is aligned at 256kB boundaries
    //   Chunk* p = new(pos) Chunk(nrBricks)
    // and use explicit destruction as follows:
    //   p->~Chunk()
    // is always 256kB*sizeof(Chunk) large and fully aligned on 
    // 256kB-sizeof(Chunk) boundary.
    char* _start;         // start of the Bricks
    void* _allocation;    // Start of allocated memory
    size_t _totalSize;
    size_t _bricksSpace;
    int32_t _nrBricks;    // number of Bricks
    int32_t _nrFree;      // number of unused Bricks
    int32_t _nextUnused;  // next unused Brick
    int32_t _freeList;    // start of free list, -1 is empty
    char padding[64-(16 + 2*sizeof(size_t) + sizeof(char*) + sizeof(void*))];  
                 // achieve sizeof(Chunk)==64

    void* brick (int32_t pos) {
      return pos == -1 ? 
             nullptr : 
             static_cast<void*>(_start + pos * Brick::totalSize);
    }

    static int32_t index (Brick* brick) {
      Chunk* chunk = brick->chunk();
      return static_cast<int32_t>
             ((reinterpret_cast<char*>(brick) - chunk->_start) 
             / Brick::totalSize);
    }

  public:

    Chunk (void* allocation, size_t nrBricks)
      : _allocation(allocation),
        _totalSize(nrBricks * Brick::totalSize + sizeof(Chunk)),
        _nrBricks(static_cast<int32_t>(nrBricks)),
        _nrFree(static_cast<int32_t>(nrBricks)) {
      _start = reinterpret_cast<char*>(this) + sizeof(Chunk);
      _nextUnused = 0;
      _freeList = -1;
    }

    ~Chunk () {
      TRI_ASSERT(empty());
    }
    
    void* allocation () {
      return _allocation;
    }

    Brick* leaseBrick (size_t blockSize) {
      // Something on the free list?
      if (_freeList == -1) {
        // No, free list is empty
        if (_nextUnused < _nrBricks) {
          _nrFree--;
          return new(brick(_nextUnused++)) Brick(blockSize, this);
        }
        else {
          return nullptr;
        }
      }
      else {
        void* res = brick(_freeList);
        int32_t* peek = static_cast<int32_t*>(res);
        _freeList = *peek;
        _nrFree--;
        return new(res) Brick(blockSize, this);
      }

    }

    bool returnBrick(Brick* brick) {
      brick->~Brick();
      int32_t* peek = reinterpret_cast<int32_t*>(brick);

      *peek = _freeList;
      _freeList = index(brick);

      _nrFree++;

      return empty();
    }

    size_t nrBricks () const {
      return _nrBricks;
    }

    size_t nrUsed () const {
      return _nrBricks - _nrFree;
    }

    size_t nrFree () const {
      return _nrFree;
    }

    bool empty () const {
      return _nrFree == _nrBricks;
    }

    bool full () const {
      return _nrFree == 0;
    }
};


// -----------------------------------------------------------------------------
// --SECTION--                                                      class Arena
// -----------------------------------------------------------------------------

Arena::~Arena () {
  _mutex.lock();
  for (Chunk* c : _fullChunks) {
    void* allocation = c->allocation();
    c->~Chunk();
    TRI_Free(TRI_UNKNOWN_MEM_ZONE, allocation);
  }
  _fullChunks.clear();
  for (Chunk* c : _notFullChunks) {
    void* allocation = c->allocation();
    c->~Chunk();
    TRI_Free(TRI_UNKNOWN_MEM_ZONE, allocation);
  }
  _notFullChunks.clear();
  _mutex.unlock();
}

Brick* Arena::leaseBrick (size_t blockSize) {
  MUTEX_LOCKER(_mutex);
  Chunk* chunk = nullptr;
  auto it = _notFullChunks.begin();
  if (it != _notFullChunks.end()) {
    chunk = *it;
  }
  else {
    void* allocation = TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, 
                                    sizeof(Chunk) + 513 * Brick::totalSize,
                                    false);
    uintptr_t u = reinterpret_cast<uintptr_t>(allocation);
    u = ((u + sizeof(Chunk) + Brick::totalSize - 1) & Brick::mask)
        - sizeof(Chunk);
    chunk = new(reinterpret_cast<void*>(u)) Chunk(allocation, 512);
    _notFullChunks.insert(chunk);
  }
  Brick* res = chunk->leaseBrick(blockSize);
  if (chunk->full()) {
    auto it = _notFullChunks.find(chunk);
    TRI_ASSERT(it != _notFullChunks.end());
    _notFullChunks.erase(it);
    _fullChunks.insert(chunk);
  }
  return res;
}

void Arena::returnBrick (Brick* brick) {
  MUTEX_LOCKER(_mutex);
  Chunk* chunk = brick->chunk();
  if (chunk->full()) {
    auto it = _fullChunks.find(chunk);
    if (it != _fullChunks.end()) {
      _fullChunks.erase(it);
    }
    _notFullChunks.insert(chunk);
  }
  if (chunk->returnBrick(brick)) {
    auto it = _notFullChunks.find(chunk);
    if (it != _notFullChunks.end()) {
      _notFullChunks.erase(it);
    }
    void* allocation = chunk->allocation();
    chunk->~Chunk();
    TRI_Free(TRI_UNKNOWN_MEM_ZONE, allocation);
  }
}


// -----------------------------------------------------------------------------
// --SECTION--                                                  class Allocator
// -----------------------------------------------------------------------------

thread_local Brick* Allocator::_current = nullptr;

Allocator::~Allocator () {
  for (Brick* b : _isFull) {
    _arena->returnBrick(b);
  }
  _isFull.clear();

  for (Brick* b : _notFull) {
    _arena->returnBrick(b);
  }
  _notFull.clear();

  for (Brick* b : _isCurrent) {
    _arena->returnBrick(b);
  }
  _isCurrent.clear();
}

void* Allocator::allocate () {
  if (_current == nullptr) {
    MUTEX_LOCKER(_mutex);
    auto it = _notFull.begin();
    if (it != _notFull.end()) {
      _current = *it;
      _notFull.erase(it);
      _isCurrent.insert(_current);
    }
    else {
      _current = _arena->leaseBrick(_blockSize);
      _isCurrent.insert(_current);
    }
  }
  void* res = _current->allocate();
  if (_current->full()) {
    MUTEX_LOCKER(_mutex);
    _isFull.insert(_current);
    auto it = _isCurrent.find(_current);
    TRI_ASSERT(it != _isCurrent.end());
    _isCurrent.erase(it);
    _current = nullptr;
  }
  return res;
}

void Allocator::free (void* block) {
  Brick* brick = Brick::findBrick(block);
  if (brick->full()) {
    MUTEX_LOCKER(_mutex);
    _notFull.insert(brick);
    auto it = _isFull.find(brick);
    TRI_ASSERT(it != _isFull.end());
    _isFull.erase(it);
  }
  brick->free(block);
  if (brick->empty()) {
    MUTEX_LOCKER(_mutex);
    auto it = _notFull.find(brick);
    if (it != _notFull.end()) {
      // Note: could be a _current
      _notFull.erase(it);
    }
    _arena->returnBrick(brick);
    if (brick == _current) {
      _current = nullptr;
    }
  }
}

