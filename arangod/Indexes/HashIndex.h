////////////////////////////////////////////////////////////////////////////////
/// @brief hash index
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
/// @author Dr. Frank Celler
/// @author Copyright 2014, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGODB_INDEXES_HASH_INDEX_H
#define ARANGODB_INDEXES_HASH_INDEX_H 1

#include "Basics/Common.h"
#include "HashIndex/hash-array.h"
#include "HashIndex/hash-array-multi.h"
#include "Indexes/Index.h"
#include "ShapedJson/shaped-json.h"
#include "VocBase/vocbase.h"
#include "VocBase/voc-types.h"

// -----------------------------------------------------------------------------
// --SECTION--                                                   class HashIndex
// -----------------------------------------------------------------------------

namespace triagens {
  namespace arango {

    class HashIndex : public Index {

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

      public:

        HashIndex () = delete;

        HashIndex (TRI_idx_iid_t,
                   struct TRI_document_collection_t*,
                   std::vector<std::string> const&,
                   std::vector<TRI_shape_pid_t> const&,
                   bool,
                   bool);

        ~HashIndex ();

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

      public:
        
        IndexType type () const override final {
          return Index::TRI_IDX_TYPE_HASH_INDEX;
        }

        bool hasSelectivityEstimate () const override final {
          return true;
        }

        double selectivityEstimate () const override final;

        bool dumpFields () const override final {
          return true;
        }
        
        size_t memory () const override final;

        triagens::basics::Json toJson (TRI_memory_zone_t*) const override final;
  
        int insert (struct TRI_doc_mptr_t const*, bool) override final;
         
        int remove (struct TRI_doc_mptr_t const*, bool) override final;
        
        int sizeHint (size_t) override final;

        int sizeHint () override final;
        
        std::vector<TRI_shape_pid_t> const& paths () const {
          return _paths;
        }

        struct TRI_document_collection_t* collection () const {
          return _collection;
        }

        bool sparse () const {
          return _sparse;
        }
        
        bool unique () const {
          return _unique;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the memory needed for an index key entry
////////////////////////////////////////////////////////////////////////////////

        size_t keyEntrySize () const {
          return _paths.size() * sizeof(TRI_shaped_json_t);
        }

        TRI_vector_pointer_t lookup (TRI_index_search_value_t*) const;

////////////////////////////////////////////////////////////////////////////////
/// @brief locates entries in the hash index given shaped json objects
////////////////////////////////////////////////////////////////////////////////

        int lookup (TRI_index_search_value_t*,
                    std::vector<TRI_doc_mptr_copy_t>&) const;

////////////////////////////////////////////////////////////////////////////////
/// @brief locates entries in the hash index given shaped json objects
////////////////////////////////////////////////////////////////////////////////

        int lookup (TRI_index_search_value_t*,
                    std::vector<TRI_doc_mptr_copy_t>&,
                    struct TRI_hash_index_element_multi_s*&,
                    size_t batchSize) const;

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

      private:

        int insertUnique (struct TRI_doc_mptr_t const*, bool);

        int insertMulti (struct TRI_doc_mptr_t const*, bool);
        
        int removeUnique (struct TRI_doc_mptr_t const*);

        int removeMulti (struct TRI_doc_mptr_t const*);

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief the attribute paths
////////////////////////////////////////////////////////////////////////////////
        
        std::vector<TRI_shape_pid_t> const  _paths;

////////////////////////////////////////////////////////////////////////////////
/// @brief the actual hash index
////////////////////////////////////////////////////////////////////////////////
  
        union {
          TRI_hash_array_t       _hashArray;        // the hash array itself, unique values
          TRI_hash_array_multi_t _hashArrayMulti;   // the hash array itself, non-unique values
        };

////////////////////////////////////////////////////////////////////////////////
/// @brief whether the index is unique
////////////////////////////////////////////////////////////////////////////////

        bool const _unique;

////////////////////////////////////////////////////////////////////////////////
/// @brief whether the index is sparse
////////////////////////////////////////////////////////////////////////////////

        bool const _sparse;
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
