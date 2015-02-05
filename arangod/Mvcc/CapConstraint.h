////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC cap constraint
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

#ifndef ARANGODB_MVCC_CAP_CONSTRAINT_H
#define ARANGODB_MVCC_CAP_CONSTRAINT_H 1

#include "Basics/Common.h"
#include "Mvcc/Index.h"

struct TRI_doc_mptr_t;
struct TRI_document_collection_t;
struct TRI_json_t;

namespace triagens {
  namespace mvcc {

    class TransactionCollection;

// -----------------------------------------------------------------------------
// --SECTION--                                               class CapConstraint
// -----------------------------------------------------------------------------

    class CapConstraint : public Index {

      public:

        CapConstraint (TRI_idx_iid_t id,
                       struct TRI_document_collection_t*,
                       size_t count,
                       int64_t size);

        ~CapConstraint ();

      public:
  
        void insert (TransactionCollection*, 
                     Transaction*,
                     struct TRI_doc_mptr_t*) = 0;
        struct TRI_doc_mptr_t* remove (TransactionCollection*,
                                       Transaction*,
                                       std::string const&,
                                       struct TRI_doc_mptr_t const*) = 0;
        void forget (TransactionCollection*,
                     Transaction*,
                     struct TRI_doc_mptr_t const*) = 0;

        void preCommit (TransactionCollection*,
                        Transaction*) = 0;

        bool hasSelectivity () const override final {
          return false;
        }

        double getSelectivity () const override final {
          return -1.0;
        }

        size_t memory () override final;
        triagens::basics::Json toJson (TRI_memory_zone_t*) const override final;

        TRI_idx_type_e type () const override final {
          return TRI_IDX_TYPE_CAP_CONSTRAINT;
        }
        
        std::string typeName () const override final {
          return "cap";
        }

      private:

        int initialize ();
        int apply (struct TRI_transaction_collection_s*);

      private:
  
        size_t const _count;
        int64_t const _size;
  
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
