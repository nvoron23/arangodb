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
#include "Basics/JsonHelper.h"
#include "Basics/AssocMulti.h"
#include "Mvcc/Index.h"
#include "VocBase/edge-collection.h"

struct TRI_doc_mptr_t;
struct TRI_document_collection_t;

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

      typedef triagens::basics::AssocMulti<struct TRI_edge_header_s, struct TRI_doc_mptr_t, uint32_t> TRI_EdgeIndexHash_t;

      public:

        EdgeIndex (TRI_idx_iid_t id,
                  struct TRI_document_collection_t*);

        ~EdgeIndex ();

      public:
  
        void insert (TransactionCollection*,
                     Transaction*,
                     struct TRI_doc_mptr_t*);
        struct TRI_doc_mptr_t* remove (TransactionCollection*,
                                       Transaction*,
                                       std::string const&,
                                       struct TRI_doc_mptr_t*);
        void forget (TransactionCollection*,
                     Transaction*,
                     struct TRI_doc_mptr_t*);

        void preCommit (TransactionCollection*,
                        Transaction*);

        std::vector<TRI_doc_mptr_t*>* lookup (TransactionCollection* coll,
                                              Transaction* trans,
                                              TRI_edge_direction_e direction,
                                              TRI_edge_header_t const* lookup,
                                              size_t limit);

        std::vector<TRI_doc_mptr_t*>* lookupContinue(
                                          TransactionCollection* coll,
                                          Transaction* trans,
                                          TRI_edge_direction_e direction,
                                          TRI_doc_mptr_t* previousLast,
                                          size_t limit);

        // give index a hint about the expected size
        void sizeHint (size_t) override final;
  
        bool hasSelectivity () const override final {
          return true;
        }

        double getSelectivity () const override final {
          return 1.0;
        }

        size_t memory () override final;
        triagens::basics::Json toJson (TRI_memory_zone_t*) const override final;

        TRI_idx_type_e type () const override final {
          return TRI_IDX_TYPE_EDGE_INDEX;
        }
        
        std::string typeName () const override final {
          return "edge";
        }

      private:

        std::vector<TRI_doc_mptr_t*>* lookupInternal(
                                          TransactionCollection* coll,
                                          Transaction* trans,
                                          TRI_edge_direction_e direction,
                                          TRI_edge_header_t const* lookup,
                                          TRI_doc_mptr_t* previousLast,
                                          size_t limit);

        void lookupPart (TRI_EdgeIndexHash_t* part,
                         Transaction* trans,
                         TRI_edge_header_t const* lookup,
                         std::vector<TRI_doc_mptr_t*>* result,
                         size_t limit);
  
        TRI_EdgeIndexHash_t* _from;
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
