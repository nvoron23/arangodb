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
#include "Basics/JsonHelper.h"
#include "Basics/AssocMulti.h"
#include "Mvcc/Index.h"

struct TRI_doc_mptr_t;
struct TRI_document_collection_t;

namespace triagens {
  namespace mvcc {

// -----------------------------------------------------------------------------
// --SECTION--                                                class PrimaryIndex
// -----------------------------------------------------------------------------

    class PrimaryIndex : public Index {
      
      public:

        PrimaryIndex (TRI_idx_iid_t id,
                      struct TRI_document_collection_t*);

        ~PrimaryIndex ();

      public:
  
        virtual void insert (class TransactionCollection*, 
                             struct TRI_doc_mptr_t const*) override final;
        virtual void remove (class TransactionCollection*,
                             struct TRI_doc_mptr_t const*) override final;
        virtual void forget (class TransactionCollection*,
                             struct TRI_doc_mptr_t const*) override final;

        virtual void preCommit (class TransactionCollection*) override final;

        // a garbage collection function for the index
        void cleanup () override final;

        // give index a hint about the expected size
        void sizeHint (size_t) override final;
  
        size_t memory () override final;
        triagens::basics::Json toJson (TRI_memory_zone_t*) const override final;

        TRI_idx_type_e type () const override final {
          return TRI_IDX_TYPE_PRIMARY_INDEX;
        }
        
        std::string typeName () const override final {
          return "primary";
        }

      private:

        typedef triagens::basics::AssocMulti<char const, 
                                             struct TRI_doc_mptr_t const,
                                             uint32_t>
                PrimaryIndexHash_t;

        PrimaryIndexHash_t* _theHash;
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
