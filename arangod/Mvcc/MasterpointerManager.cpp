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

#include "MasterpointerManager.h"
#include "Basics/MutexLocker.h"
#include "VocBase/document-collection.h"

using namespace triagens::basics;
using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                     struct MasterpointerContainer
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief move constructor for master pointer container
/// moves the master pointer from the other container into ourselves
////////////////////////////////////////////////////////////////////////////////

MasterpointerContainer::MasterpointerContainer (MasterpointerContainer&& other) 
  : manager(other.manager),
    mptr(other.mptr),
    owns(other.owns) {
  other.mptr = nullptr;
  other.owns = false;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief master pointer container constructor
////////////////////////////////////////////////////////////////////////////////
        
MasterpointerContainer::MasterpointerContainer (MasterpointerManager* manager,
                                                TRI_doc_mptr_t* mptr)
  : manager(manager),
    mptr(mptr),
    owns(true) {

}

////////////////////////////////////////////////////////////////////////////////
/// @brief master pointer container destructor
////////////////////////////////////////////////////////////////////////////////

MasterpointerContainer::~MasterpointerContainer () {
  if (owns && mptr != nullptr) {
    manager->recycle(mptr);
    owns = false;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief tell the master pointer manager to link the master pointer
/// this will also transfer ownership back to the master pointer manager
////////////////////////////////////////////////////////////////////////////////
        
void MasterpointerContainer::link () {
  if (! owns) {
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_INTERNAL, "should not link a non-owned master pointer");
  }

  owns = false;
  manager->link(mptr);
}

// -----------------------------------------------------------------------------
// --SECTION--                                        class MasterpointerManager
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief creates the master pointer manager
////////////////////////////////////////////////////////////////////////////////

MasterpointerManager::MasterpointerManager () 
  : _lock(),
    _freelist(nullptr),
    _blocks() {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroys the master pointer manager
////////////////////////////////////////////////////////////////////////////////

MasterpointerManager::~MasterpointerManager () {
  for (auto it : _blocks) {
    delete[] it;
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new master pointer in the collection
/// the master pointer is not yet linked
////////////////////////////////////////////////////////////////////////////////

MasterpointerContainer MasterpointerManager::create (void const* data,
                                                     triagens::mvcc::TransactionId::IdType transactionId) {
  TRI_doc_mptr_t* mptr = nullptr;

  {
    MUTEX_LOCKER(_lock);

    if (_freelist == nullptr) {
      size_t const blockSize = getBlockSize(_blocks.size());

      // acquire a new block
      auto begin = new TRI_doc_mptr_t[blockSize];
      try {
        _blocks.push_back(begin);
      }
      catch (...) {
        delete[] begin;
        throw;
      }

      // initialize all acquired master pointers
      TRI_doc_mptr_t* header = nullptr;

      TRI_doc_mptr_t* ptr = begin + (blockSize - 1);
      for (;  begin <= ptr;  ptr--) {
        ptr->setDataPtr(header); // ONLY IN HEADERS
        header = ptr;
      }
    
      _freelist = header;
    }

    mptr = _freelist;
    _freelist = const_cast<TRI_doc_mptr_t*>(static_cast<TRI_doc_mptr_t const*>(mptr->getDataPtr())); // ONLY IN HEADERS, PROTECTED by RUNTIME
    mptr->setDataPtr(nullptr);
  }

  // outside the lock
  TRI_ASSERT_EXPENSIVE(mptr != nullptr);

  // TODO: properly initialize the master pointer
  mptr->setDataPtr(data);
  mptr->_rid = TRI_EXTRACT_MARKER_RID(static_cast<TRI_df_marker_t const*>(data));
  // mptr->_from = transactionId;
  // mptr->_to   = 0;
  mptr->_prev = nullptr;
  mptr->_next = nullptr;
  
  return MasterpointerContainer(this, mptr);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief link the master pointer
////////////////////////////////////////////////////////////////////////////////

void MasterpointerManager::link (TRI_doc_mptr_t* mptr) {
  // TODO: 
}

////////////////////////////////////////////////////////////////////////////////
/// @brief recycle a master pointer in the collection
/// the caller must guarantee that the master pointer will not be read or
/// modified by itself or any other threads
////////////////////////////////////////////////////////////////////////////////

void MasterpointerManager::recycle (TRI_doc_mptr_t* mptr) {
  // TODO: implement
  std::cout << "RECYCLING MPTR: " << mptr << "\n";
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  private methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief get the size (number of entries) for a block, based on a function
///
/// this adaptively increases the number of entries per block until a certain
/// threshold. the benefit of this is that small collections (with few
/// documents) only use little memory whereas bigger collections allocate new
/// blocks in bigger chunks.
/// the lowest value for the number of entries in a block is BLOCK_SIZE_UNIT,
/// the highest value is BLOCK_SIZE_UNIT << 8.
////////////////////////////////////////////////////////////////////////////////

size_t MasterpointerManager::getBlockSize (size_t blockNumber) const {
  static size_t const BLOCK_SIZE_UNIT = 128;

  if (blockNumber < 8) {
    // use a small block size in the beginning to save memory
    return (size_t) (BLOCK_SIZE_UNIT << blockNumber);
  }

  // use a block size of 32768
  // this will use 32768 * sizeof(TRI_doc_mptr_t) bytes, i.e. 1.5 MB
  return (size_t) (BLOCK_SIZE_UNIT << 8);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
