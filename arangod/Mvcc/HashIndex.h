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
#include "Mvcc/Index.h"
#include "HashIndex/hash-array.h"
#include "HashIndex/hash-array-multi.h"
#include "ShapedJson/shaped-json.h"

struct TRI_doc_mptr_t;
struct TRI_document_collection_t;
struct TRI_json_t;
struct TRI_transaction_collection_s;

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
                   bool unique);

        ~HashIndex ();

      public:
  
        int insert (struct TRI_doc_mptr_t const*, bool) override final;
        int remove (struct TRI_doc_mptr_t const*, bool) override final;
        int postInsert (struct TRI_transaction_collection_s*, struct TRI_doc_mptr_t const*) override final;

        // a garbage collection function for the index
        int cleanup () override final;

        // give index a hint about the expected size
        int sizeHint (size_t) override final;
  
        size_t memory () override final;
        struct TRI_json_t* toJson (TRI_memory_zone_t*) const override final;

        TRI_idx_type_e type () const override final {
          return TRI_IDX_TYPE_HASH_INDEX;
        }
        
        std::string typeName () const override final {
          return "hash";
        }

      private:

        size_t keyEntrySize () const;

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

      private:

        std::vector<TRI_shape_pid_t> const  _paths;

        union {
          TRI_hash_array_t       _hashArray;   // the hash array itself, unique values
          TRI_hash_array_multi_t _hashArrayMulti;   // the hash array itself, non-unique values
        };


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
