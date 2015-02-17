////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC hash index
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

#ifndef ARANGODB_MVCC_HASH_INDEX_H
#define ARANGODB_MVCC_HASH_INDEX_H 1

#include "Basics/Common.h"
#include "Basics/JsonHelper.h"
#include "Mvcc/Index.h"
#include "HashIndex/hash-array.h"
#include "HashIndex/hash-array-multi.h"
#include "ShapedJson/shaped-json.h"

struct TRI_doc_mptr_t;
struct TRI_document_collection_t;

namespace triagens {
  namespace mvcc {

// -----------------------------------------------------------------------------
// --SECTION--                                                   class HashIndex
// -----------------------------------------------------------------------------

    class HashIndex : public Index {
      
      public:

        struct Element {
          TRI_doc_mptr_t*  _document;
          TRI_shaped_sub_t _subObjects[];
        };

        typedef std::vector<TRI_shaped_json_t> Key;

        HashIndex (TRI_idx_iid_t id,
                   struct TRI_document_collection_t*,
                   std::vector<std::string> const& fields,
                   std::vector<TRI_shape_pid_t> const& paths,
                   bool unique,
                   bool sparse);

        ~HashIndex ();

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

        std::vector<TRI_doc_mptr_t*>* lookup (TransactionCollection* coll,
                                              Transaction* trans,
                                              Key const* key,
                                              size_t limit);

        std::vector<TRI_doc_mptr_t*>* lookupContinue(
                                          TransactionCollection* coll,
                                          Transaction* trans,
                                          TRI_doc_mptr_t* previousLast,
                                          size_t limit);

        // a garbage collection function for the index
        void cleanup () override final;

        // give index a hint about the expected size
        void sizeHint (size_t) override final;
  
        bool hasSelectivity () const override final {
          return true;
        }

        double getSelectivity () const override final {
          return _theHash->selectivity();
        }

        size_t memory () override final;
        triagens::basics::Json toJson (TRI_memory_zone_t*) const override final;

        TRI_idx_type_e type () const override final {
          return TRI_IDX_TYPE_HASH_INDEX;
        }
        
        std::string typeName () const override final {
          return "hash";
        }

        size_t nrIndexedFields () const {
          return _paths.size();
        }

        std::vector<TRI_shape_pid_t> const& paths () const {
          return _paths;
        }

      private:

        std::vector<TRI_doc_mptr_t*>* lookupInternal (
                                          TransactionCollection* coll,
                                          Transaction* trans,
                                          Key const* key,
                                          TRI_doc_mptr_t* previousLast,
                                          size_t limit);
        size_t elementSize () const;

        Element* allocateAndFillElement (TRI_doc_mptr_t* doc,
                                         bool& includeForSparse);

        void deleteElement (Element*);

        std::vector<TRI_shape_pid_t> const  _paths;

        typedef triagens::basics::AssocMulti<Key, Element, uint32_t> Hash_t;

        Hash_t* _theHash;

        bool _unique;

        bool _sparse;
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
