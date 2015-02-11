////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC index base class
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

#ifndef ARANGODB_MVCC_INDEX_H
#define ARANGODB_MVCC_INDEX_H 1

#include "Basics/Common.h"
#include "Basics/ReadWriteLock.h"
#include "Basics/JsonHelper.h"
#include "VocBase/index.h"
#include "VocBase/voc-types.h"

struct TRI_doc_mptr_t;
struct TRI_document_collection_t;
struct TRI_json_t;

namespace triagens {
  namespace mvcc {

// -----------------------------------------------------------------------------
// --SECTION--                                                       class Index
// -----------------------------------------------------------------------------

    class TransactionCollection;
    class Transaction;

    class Index {

      protected:

        Index (Index const&) = delete;
        Index& operator= (Index const&) = delete;

        Index (TRI_idx_iid_t id,
               struct TRI_document_collection_t*,
               std::vector<std::string> const& fields);

      public:

        virtual ~Index ();

      public:

        virtual void insert (TransactionCollection*,
                             Transaction*,
                             struct TRI_doc_mptr_t*) = 0;
        virtual struct TRI_doc_mptr_t* remove (
                         TransactionCollection*,
                         Transaction*,
                         std::string const&,
                         struct TRI_doc_mptr_t*) = 0;
        virtual void forget (TransactionCollection*,
                             Transaction*,
                             struct TRI_doc_mptr_t*) = 0;

        virtual void preCommit (TransactionCollection*,
                                Transaction*) = 0;

        virtual size_t memory () = 0;
        virtual TRI_idx_type_e type () const = 0;
        virtual std::string typeName () const = 0;
        
        // a garbage collection function for the index
        virtual void cleanup ();
        virtual void sizeHint (size_t);
        
        virtual bool hasSelectivity () const = 0;
        virtual double getSelectivity () const = 0;

        virtual triagens::basics::Json figures () const;

        virtual triagens::basics::Json toJson (TRI_memory_zone_t*) const;

        inline TRI_idx_iid_t id () const {
          return _id;
        }

        inline void clickLock (void) {
          _lock.writeLock();
          _lock.writeUnlock();
        }

      protected:

        TRI_idx_iid_t const               _id;
  
        struct TRI_document_collection_t* _collection;

        std::vector<std::string> const    _fields;

        triagens::basics::ReadWriteLock   _lock;
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
