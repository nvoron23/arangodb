////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC edge index
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

#ifndef ARANGODB_MVCC_EDGE_INDEX_H
#define ARANGODB_MVCC_EDGE_INDEX_H 1

#include "Basics/Common.h"
#include "Basics/AssocMulti.h"
#include "Basics/JsonHelper.h"
#include "Basics/ReadWriteLock.h"
#include "Mvcc/Index.h"
#include "VocBase/voc-types.h"

struct TRI_document_collection_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief edge direction
////////////////////////////////////////////////////////////////////////////////

typedef enum {
  TRI_EDGE_ANY    = 0, // can only be used for searching
  TRI_EDGE_IN     = 1,
  TRI_EDGE_OUT    = 2
}
TRI_edge_direction_e;

////////////////////////////////////////////////////////////////////////////////
/// @brief index entry for edges
////////////////////////////////////////////////////////////////////////////////

typedef struct TRI_edge_header_s {
  TRI_voc_cid_t _cid; // from or to, depending on the direction
  TRI_voc_key_t _key;
}
TRI_edge_header_t;


namespace triagens {
  namespace mvcc {

    class TransactionCollection;
    class Transaction;

    struct EdgeIndexSearchValue {
      TRI_voc_key_t key;
      TRI_voc_cid_t cid;
      TRI_edge_direction_e direction;
    };

// -----------------------------------------------------------------------------
// --SECTION--                                                   class EdgeIndex
// -----------------------------------------------------------------------------

    class EdgeIndex : public Index {

// -----------------------------------------------------------------------------
// --SECTION--                                                     private types
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief typedef for the hash tables
////////////////////////////////////////////////////////////////////////////////
      
        typedef triagens::basics::AssocMulti<struct TRI_edge_header_s, 
                                             struct TRI_doc_mptr_t, 
                                             uint32_t> TRI_EdgeIndexHash_t;

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------
      
      public:
        
        EdgeIndex (TRI_idx_iid_t);

        EdgeIndex (TRI_idx_iid_t,
                  struct TRI_document_collection_t*);

        ~EdgeIndex ();

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
          return TRI_IDX_TYPE_EDGE_INDEX;
        }
        
        std::string typeName () const override final {
          return "edge";
        }
        
        void clickLock () override final {
          _lock.writeLock();
          _lock.writeUnlock();
        }

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------
        
      public:

        std::vector<TRI_doc_mptr_t*>* lookup (Transaction*,
                                              TRI_edge_direction_e,
                                              TRI_edge_header_t const*,
                                              size_t);

        std::vector<TRI_doc_mptr_t*>* lookupContinue (Transaction*,
                                                      TRI_edge_direction_e,
                                                      TRI_doc_mptr_t*,
                                                      size_t);

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

      private:

        std::vector<TRI_doc_mptr_t*>* lookupInternal (Transaction*,
                                                      TRI_edge_direction_e,
                                                      TRI_edge_header_t const*,
                                                      TRI_doc_mptr_t*,
                                                      size_t);

        void lookupPart (TRI_EdgeIndexHash_t*,
                         Transaction*,
                         TRI_edge_header_t const*,
                         std::vector<TRI_doc_mptr_t*>*,
                         size_t,
                         bool);

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------
      
      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief the underlying collection
////////////////////////////////////////////////////////////////////////////////
  
        struct TRI_document_collection_t* _collection;

////////////////////////////////////////////////////////////////////////////////
/// @brief the index R/W lock
////////////////////////////////////////////////////////////////////////////////
        
        triagens::basics::ReadWriteLock   _lock;

////////////////////////////////////////////////////////////////////////////////
/// @brief hash table for _from
////////////////////////////////////////////////////////////////////////////////
  
        TRI_EdgeIndexHash_t* _from;

////////////////////////////////////////////////////////////////////////////////
/// @brief hash table for _to
////////////////////////////////////////////////////////////////////////////////

        TRI_EdgeIndexHash_t* _to;
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
