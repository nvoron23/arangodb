////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC primary index
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
/// @author Max Neunhoeffer
/// @author Copyright 2015, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGODB_MVCC_PRIMARY_INDEX_H
#define ARANGODB_MVCC_PRIMARY_INDEX_H 1

#include "Basics/Common.h"
#include "Basics/fasthash.h"
#include "Basics/AssocMulti.h"
#include "Basics/JsonHelper.h"
#include "Basics/ReadLocker.h"
#include "Basics/ReadWriteLock.h"
#include "Mvcc/Index.h"
#include "Mvcc/TransactionId.h"
#include "Mvcc/Transaction.h"

struct TRI_document_collection_t;

namespace triagens {
  namespace mvcc {

// -----------------------------------------------------------------------------
// --SECTION--                                                class PrimaryIndex
// -----------------------------------------------------------------------------

    class PrimaryIndex : public Index {
    
      friend class PrimaryIndexReadAccessor;

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------
      
      public:
        
        PrimaryIndex ();

        PrimaryIndex (TRI_idx_iid_t id,
                      struct TRI_document_collection_t*);

        ~PrimaryIndex ();

// -----------------------------------------------------------------------------
// --SECTION--                            public methods, inherited from Index.h
// -----------------------------------------------------------------------------

      public:

        void insert (struct TRI_doc_mptr_t*) override final;
        
        void insert (TransactionCollection*, 
                     Transaction*,
                     struct TRI_doc_mptr_t*) override final;

        struct TRI_doc_mptr_t* remove (TransactionCollection*,
                                       Transaction*,
                                       std::string const&,
                                       struct TRI_doc_mptr_t*) override final;
        
        void forget (TransactionCollection*,
                     Transaction*,
                     struct TRI_doc_mptr_t*) override final;

        void preCommit (TransactionCollection*,
                        Transaction*) override final;
        
        void cleanup () override final;

        void sizeHint (size_t) override final;
  
        bool hasSelectivity () const override final {
          return true;
        }

        double getSelectivity () const override final {
          return 1.0;
        }

        size_t memory () override final;

        triagens::basics::Json toJson (TRI_memory_zone_t*) const override final;

        Index::IndexType type () const override final {
          return TRI_IDX_TYPE_PRIMARY_INDEX;
        }
        
        std::string typeName () const override final {
          return "primary";
        }

        void clickLock () override final {
          _lock.writeLock();
          _lock.writeUnlock();
        }

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief return the number of documents in the index
////////////////////////////////////////////////////////////////////////////////

        size_t size ();

////////////////////////////////////////////////////////////////////////////////
/// @brief resizes the primary index to the specified target size
////////////////////////////////////////////////////////////////////////////////
    
        void resize (uint32_t);

////////////////////////////////////////////////////////////////////////////////
/// @brief return a hash value for a key
////////////////////////////////////////////////////////////////////////////////
        
        static uint64_t hashKeyString (char const* key, size_t len) {
          return fasthash64(key, len, 0x13579864);
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief remove a document from the index. this is called only when opening
/// a collection, without concurrency
////////////////////////////////////////////////////////////////////////////////
        
        struct TRI_doc_mptr_t* remove (std::string const&);

////////////////////////////////////////////////////////////////////////////////
/// @brief lookup a document in the index. this is called only when opening
/// a collection, without concurrency
////////////////////////////////////////////////////////////////////////////////
        
        struct TRI_doc_mptr_t* lookup (std::string const&);
        
////////////////////////////////////////////////////////////////////////////////
/// @brief iterate over all documents in the index. this is called only when 
/// opening a collection, without concurrency
////////////////////////////////////////////////////////////////////////////////

        void iterate (std::function<void(struct TRI_doc_mptr_t*)>);

////////////////////////////////////////////////////////////////////////////////
/// @brief lookup method with non-standard signature. this is called for regular
/// document insertions
////////////////////////////////////////////////////////////////////////////////
        
        struct TRI_doc_mptr_t* lookup (TransactionCollection*,
                                       Transaction*,
                                       std::string const& key);

////////////////////////////////////////////////////////////////////////////////
/// @brief remove method with non-standard signature. this is called for regular
/// document removals
////////////////////////////////////////////////////////////////////////////////
        
        struct TRI_doc_mptr_t* remove (TransactionCollection*,
                                       Transaction*,
                                       std::string const&,
                                       TransactionId&);

////////////////////////////////////////////////////////////////////////////////
/// @brief returns a random document from the index
////////////////////////////////////////////////////////////////////////////////
        
        struct TRI_doc_mptr_t* random (TransactionCollection*,
                                       Transaction*);

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief returns all relevant revisions in a write-operation context
////////////////////////////////////////////////////////////////////////////////

        TRI_doc_mptr_t* findRelevantRevisionForWrite (TransactionCollection*,
                                                      Transaction*,
                                                      std::string const&,
                                                      bool&) const;

////////////////////////////////////////////////////////////////////////////////
/// @brief returns a visible revision for the given key or a nullptr if none
////////////////////////////////////////////////////////////////////////////////

        TRI_doc_mptr_t* findVisibleRevision (TransactionCollection*,
                                             Transaction*,
                                             std::string const&) const;

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------
      
      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief the underlying collection
////////////////////////////////////////////////////////////////////////////////
  
        struct TRI_document_collection_t* _collection;

////////////////////////////////////////////////////////////////////////////////
/// @brief typedef for the hash table
////////////////////////////////////////////////////////////////////////////////
        
        typedef triagens::basics::AssocMulti<std::string const,
                                             struct TRI_doc_mptr_t,
                                             uint32_t> PrimaryIndexHash_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief the index R/W lock
////////////////////////////////////////////////////////////////////////////////
        
        triagens::basics::ReadWriteLock _lock;

////////////////////////////////////////////////////////////////////////////////
/// @brief the underlying hash table
////////////////////////////////////////////////////////////////////////////////

        PrimaryIndexHash_t* _theHash;

    };

// -----------------------------------------------------------------------------
// --SECTION--                                    class PrimaryIndexReadAccessor
// -----------------------------------------------------------------------------

    class PrimaryIndexReadAccessor {

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a read accessor. the accessor will hold the read-lock on
/// the index during the accessor's lifetime
////////////////////////////////////////////////////////////////////////////////
       
        PrimaryIndexReadAccessor (PrimaryIndex*);

////////////////////////////////////////////////////////////////////////////////
/// @brief destroys read accessor. this will also release the read-lock on the 
/// index
////////////////////////////////////////////////////////////////////////////////

        ~PrimaryIndexReadAccessor ();

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief looks up a key in the primary index while holding the read-lock
////////////////////////////////////////////////////////////////////////////////
        
        struct TRI_doc_mptr_t* lookup (TransactionCollection*,
                                       Transaction*,
                                       std::string const& key);

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief the primary index
////////////////////////////////////////////////////////////////////////////////

        PrimaryIndex* _index;

////////////////////////////////////////////////////////////////////////////////
/// @brief a read-locker for the index lock
////////////////////////////////////////////////////////////////////////////////

        triagens::basics::ReadLocker const _readLocker;
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
