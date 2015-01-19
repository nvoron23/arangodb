////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC fulltext index
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

#ifndef ARANGODB_MVCC_FULLTEXT_INDEX_H
#define ARANGODB_MVCC_FULLTEXT_INDEX_H 1

#include "Basics/Common.h"
#include "Basics/JsonHelper.h"
#include "Mvcc/Index.h"
#include "FulltextIndex/fulltext-index.h"
#include "ShapedJson/shaped-json.h"

struct TRI_doc_mptr_t;
struct TRI_document_collection_t;
struct TRI_fulltext_wordlist_s;
struct TRI_json_t;
struct TRI_transaction_collection_s;

namespace triagens {
  namespace mvcc {

// -----------------------------------------------------------------------------
// --SECTION--                                               class FulltextIndex
// -----------------------------------------------------------------------------

    class FulltextIndex : public Index {

      public:

        FulltextIndex (TRI_idx_iid_t id,
                       struct TRI_document_collection_t*,
                       std::vector<std::string> const& fields,
                       int minWordLength);

        ~FulltextIndex ();

      public:
  
        virtual void insert (class TransactionCollection*, 
                             struct TRI_doc_mptr_t const*) override final;
        virtual void remove (class TransactionCollection*,
                             struct TRI_doc_mptr_t const*) override final;
        virtual void forget (class TransactionCollection*,
                             struct TRI_doc_mptr_t const*) override final;

        virtual void preCommit (class TransactionCollection*) override final;

        void cleanup () override final;
        size_t memory () override final;
        triagens::basics::Json toJson (TRI_memory_zone_t*) const override final;

        TRI_idx_type_e type () const override final {
          return TRI_IDX_TYPE_FULLTEXT_INDEX;
        }
        
        std::string typeName () const override final {
          return "fulltext";
        }

      private:

        struct TRI_fulltext_wordlist_s* getWordlist (struct TRI_doc_mptr_t const*);

      private:
  
        TRI_fts_index_t* _fulltextIndex;
        TRI_shape_pid_t _attribute;
        int _minWordLength;

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
