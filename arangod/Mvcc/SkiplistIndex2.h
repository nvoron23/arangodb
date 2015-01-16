////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC skiplist index
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

#ifndef ARANGODB_MVCC_SKIPLIST_INDEX_H
#define ARANGODB_MVCC_SKIPLIST_INDEX_H 1

#include "Basics/Common.h"
#include "Mvcc/Index.h"
#include "SkipLists/skiplistIndex.h"
#include "ShapedJson/shaped-json.h"

struct TRI_doc_mptr_t;
struct TRI_document_collection_t;
struct TRI_json_t;
struct TRI_transaction_collection_s;

namespace triagens {
  namespace mvcc {

// -----------------------------------------------------------------------------
// --SECTION--                                               class SkiplistIndex
// -----------------------------------------------------------------------------

    class SkiplistIndex2 : public Index {
      
      public:

        SkiplistIndex2 (TRI_idx_iid_t id,
                        struct TRI_document_collection_t*,
                        std::vector<std::string> const& fields,
                        std::vector<TRI_shape_pid_t> const& paths,
                        bool unique,
                        bool ignoreNull,
                        bool sparse);

        ~SkiplistIndex2 ();

      public:
  
        int insert (struct TRI_doc_mptr_t const*, bool) override final;
        int remove (struct TRI_doc_mptr_t const*, bool) override final;
        int postInsert (struct TRI_transaction_collection_s*, struct TRI_doc_mptr_t const*) override final;

        size_t memory () override final;
        struct TRI_json_t* toJson (TRI_memory_zone_t*) const override final;

        TRI_idx_type_e type () const override final {
          return TRI_IDX_TYPE_SKIPLIST_INDEX;
        }
        
        std::string typeName () const override final {
          return "skiplist";
        }

      private:

       int shapify (TRI_skiplist_index_element_t*,
                    struct TRI_doc_mptr_t const*);

      private:

        std::vector<TRI_shape_pid_t> _paths;  
        SkiplistIndex*               _skiplistIndex; 

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
