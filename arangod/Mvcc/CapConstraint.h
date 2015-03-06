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

struct TRI_document_collection_t;

namespace triagens {
  namespace mvcc {

    class Transaction;
    class TransactionCollection;

// -----------------------------------------------------------------------------
// --SECTION--                                               class CapConstraint
// -----------------------------------------------------------------------------

    class CapConstraint : public Index {

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

      public:

        CapConstraint (TRI_idx_iid_t,
                       struct TRI_document_collection_t*,
                       size_t,
                       int64_t);

        ~CapConstraint ();

// -----------------------------------------------------------------------------
// --SECTION--                            public methods, inherited from Index.h
// -----------------------------------------------------------------------------

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
        
        void clickLock () override final {
        }

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

        int apply (TransactionCollection*,
                   Transaction*,
                   bool);

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief the underlying collection
////////////////////////////////////////////////////////////////////////////////
  
        struct TRI_document_collection_t* _collection;

////////////////////////////////////////////////////////////////////////////////
/// @brief maximum number of documents in the collection
////////////////////////////////////////////////////////////////////////////////
  
        int64_t const _count;

////////////////////////////////////////////////////////////////////////////////
/// @brief maximum size of documents in the collection
////////////////////////////////////////////////////////////////////////////////

        int64_t const _size;

////////////////////////////////////////////////////////////////////////////////
/// @brief minimum bytesize value for cap constraint
////////////////////////////////////////////////////////////////////////////////

      public: 

        static int64_t const MinSize;
  
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
