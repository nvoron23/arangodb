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

        HashIndex (TRI_idx_iid_t id,
                   struct TRI_document_collection_t*,
                   std::vector<std::string> const& fields,
                   std::vector<TRI_shape_pid_t> const& paths,
                   bool unique,
                   bool sparse);

        ~HashIndex ();

      public:
  
        virtual void insert (class TransactionCollection*, 
                             struct TRI_doc_mptr_t*) override final;
        virtual struct TRI_doc_mptr_t* remove (
                   class TransactionCollection*,
                   std::string const&,
                   struct TRI_doc_mptr_t const*) override final;
        virtual void forget (class TransactionCollection*,
                             struct TRI_doc_mptr_t const*) override final;

        virtual void preCommit (class TransactionCollection*) override final;

        std::vector<TRI_doc_mptr_t*>* lookup (class TransactionCollection*,
                                              Key const*) const;

        // a garbage collection function for the index
        void cleanup () override final;

        // give index a hint about the expected size
        void sizeHint (size_t) override final {
          int res = _theHash->resize(3 * size + 1);  
            // Take into account old revisions
          if (res == TRI_ERROR_OUT_OF_MEMORY) {
            THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
          }
        }
  
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

      private:

        size_t keySize () const;

        template<typename T>
        int fillIndexSearchValueByHashIndexElement (struct TRI_index_search_value_s*,
                                                    T const*);

        template<typename T>
        int allocateSubObjectsHashIndexElement (T*);

        template<typename T>
        void freeSubObjectsHashIndexElement (T*);

        template<typename T>
        int hashIndexHelper (T*, struct TRI_doc_mptr_t const*);

        template<typename T>
        int hashIndexHelperAllocate (T*, struct TRI_doc_mptr_t const*);

        int insertUnique (struct TRI_hash_index_element_s const*, bool);
        int removeUnique (struct TRI_hash_index_element_s*);
        struct TRI_hash_index_element_s* findUnique (struct TRI_index_search_value_s*);

        int insertMulti (struct TRI_hash_index_element_multi_s*, bool);
        int removeMulti (struct TRI_hash_index_element_multi_s*);

      public:

        struct Element {
          TRI_doc_mptr_t*    _document;
          TRI_shaped_sub_t[] _subObjects;
        };

        typedef std::vector<TRI_shaped_json_t> Key;

      private:

        Element* allocAndFillElement(TRI_doc_mptr_t* doc);

        void deleteElement(Element*);

        std::vector<TRI_shape_pid_t> const  _paths;

        typedef triagens::basics::AssocMulti<Key, Element, uint32_t>
                HashIndexHash_t;

        Hash_t* _theHash;

        bool _unique;

        bool _sparse;  // NOT YET IMPLEMENTED, IS SILENTLY IGNORED AT THIS STAGE
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
