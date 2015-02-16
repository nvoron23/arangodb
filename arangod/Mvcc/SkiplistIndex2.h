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
#include "Basics/JsonHelper.h"
#include "Basics/SkipList.h"
#include "Mvcc/Index.h"
#include "ShapedJson/shaped-json.h"

struct TRI_doc_mptr_t;
struct TRI_document_collection_t;
struct TRI_index_operator_s;

namespace triagens {
  namespace mvcc {

// -----------------------------------------------------------------------------
// --SECTION--                                               class SkiplistIndex
// -----------------------------------------------------------------------------

    class SkiplistIndex2 : public Index {
      
      public:

        struct Element {
          TRI_doc_mptr_t*  _document;
          TRI_shaped_sub_t _subObjects[];
        };

        typedef std::vector<TRI_shaped_json_t> Key;

        SkiplistIndex2 (TRI_idx_iid_t id,
                        struct TRI_document_collection_t*,
                        std::vector<std::string> const& fields,
                        std::vector<TRI_shape_pid_t> const& paths,
                        bool unique,
                        bool sparse);

        ~SkiplistIndex2 ();

      public:

        std::vector<TRI_doc_mptr_t*>* lookup (TransactionCollection*,
                                              Transaction*,
                                              struct TRI_index_operator_s*,
                                              bool,
                                              size_t);

        std::vector<TRI_doc_mptr_t*>* lookupContinue (TransactionCollection*,
                                              Transaction*,
                                              struct TRI_index_operator_s*,
                                              TRI_doc_mptr_t*,
                                              bool,
                                              size_t);
  
        virtual void insert (TransactionCollection*,
                             Transaction*,
                             struct TRI_doc_mptr_t*) override final;

        virtual struct TRI_doc_mptr_t* remove (
                   TransactionCollection*,
                   Transaction*,
                   std::string const&,
                   struct TRI_doc_mptr_t*) override final;

        virtual void forget (TransactionCollection*,
                             Transaction*,
                             struct TRI_doc_mptr_t*) override final;

        virtual void preCommit (TransactionCollection*,
                                Transaction*) override final;

        // a garbage collection function for the index
        void cleanup () override final;

        // give index a hint about the expected size
        void sizeHint (size_t) override final;
  
        bool hasSelectivity () const override final {
          return false;
        }

        double getSelectivity () const override final {
          return -1.0;
        }

        size_t memory () override final;
        triagens::basics::Json toJson (TRI_memory_zone_t*) const override final;

        TRI_idx_type_e type () const override final {
          return TRI_IDX_TYPE_SKIPLIST_INDEX;
        }
        
        std::string typeName () const override final {
          return "skiplist";
        }

        size_t nrIndexedFields () const {
          return _paths.size();
        }

        std::vector<TRI_shape_pid_t> const& paths () const {
          return _paths;
        }

      public:

        typedef triagens::basics::SkipList<Key, Element> SkipList_t;

        size_t elementSize () const;

        Element* allocateAndFillElement(TransactionCollection* coll,
                                        TRI_doc_mptr_t* doc,
                                        bool& includeForSparse);

        void deleteElement(Element*);

      private:

        std::vector<TRI_shape_pid_t> const  _paths;

        SkipList_t* _theSkipList;

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
