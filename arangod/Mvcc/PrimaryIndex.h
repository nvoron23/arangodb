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
#include "Basics/fasthash.h"

#include "Mvcc/Index.h"
#include "Mvcc/TransactionId.h"
#include "Mvcc/Transaction.h"

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
  
        virtual void insert (TransactionCollection*, 
                             Transaction*,
                             struct TRI_doc_mptr_t*) override final;

        virtual struct TRI_doc_mptr_t* remove (
                  TransactionCollection*,
                  Transaction*,
                  std::string const&,
                  struct TRI_doc_mptr_t*) override final;
        
        struct TRI_doc_mptr_t* remove (
                TransactionCollection*,
                Transaction*,
                std::string const&,
                TransactionId&);

        virtual void forget (TransactionCollection*,
                             Transaction*,
                             struct TRI_doc_mptr_t*) override final;

        virtual void preCommit (TransactionCollection*,
                                Transaction*) override final;
        
        struct TRI_doc_mptr_t* random (TransactionCollection*,
                                       Transaction*);

        struct TRI_doc_mptr_t* lookup (TransactionCollection*,
                                       Transaction*,
                                       std::string const& key);

        // a garbage collection function for the index
        void cleanup () override final;

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
          return TRI_IDX_TYPE_PRIMARY_INDEX;
        }
        
        std::string typeName () const override final {
          return "primary";
        }

        static uint64_t hashKeyString (char const* key, size_t len) {
          return fasthash64(key, len, 0x13579864);
        }

      private:

        typedef triagens::basics::AssocMulti<std::string const, 
                                             struct TRI_doc_mptr_t,
                                             uint32_t>
                PrimaryIndexHash_t;

        PrimaryIndexHash_t* _theHash;

        TRI_doc_mptr_t* findRelevantRevisionForWrite (
                    TransactionCollection* transColl,
                    Transaction*,
                    std::string const& key,
                    bool& writeOk) const;

        TRI_doc_mptr_t* findVisibleRevision (
                    TransactionCollection* transColl,
                    Transaction*,
                    std::string const& key) const;
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
