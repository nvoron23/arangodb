////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC index base class / interface
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
#include "Basics/JsonHelper.h"
#include "VocBase/voc-types.h"

struct TRI_doc_mptr_t;
struct TRI_json_t;

namespace triagens {
  namespace mvcc {

    class TransactionCollection;
    class Transaction;

// -----------------------------------------------------------------------------
// --SECTION--                                                       class Index
// -----------------------------------------------------------------------------

    class Index {

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief index type
////////////////////////////////////////////////////////////////////////////////

        enum IndexType {
          TRI_IDX_TYPE_UNKNOWN = 0,
          TRI_IDX_TYPE_PRIMARY_INDEX,
          TRI_IDX_TYPE_GEO1_INDEX,
          TRI_IDX_TYPE_GEO2_INDEX,
          TRI_IDX_TYPE_HASH_INDEX,
          TRI_IDX_TYPE_EDGE_INDEX,
          TRI_IDX_TYPE_FULLTEXT_INDEX,
          TRI_IDX_TYPE_PRIORITY_QUEUE_INDEX, // DEPRECATED and not functional anymore
          TRI_IDX_TYPE_SKIPLIST_INDEX,
          TRI_IDX_TYPE_BITARRAY_INDEX,       // DEPRECATED and not functional anymore
          TRI_IDX_TYPE_CAP_CONSTRAINT
        };

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

      protected:

        Index (Index const&) = delete;
        Index& operator= (Index const&) = delete;

////////////////////////////////////////////////////////////////////////////////
/// @brief create the index
////////////////////////////////////////////////////////////////////////////////

        Index (TRI_idx_iid_t,
               std::vector<std::string> const&);

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the index
////////////////////////////////////////////////////////////////////////////////

        virtual ~Index ();

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief return the index fields
////////////////////////////////////////////////////////////////////////////////

        inline std::vector<std::string> fields () const {
          return _fields;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief return the index id
////////////////////////////////////////////////////////////////////////////////
        
        inline TRI_idx_iid_t id () const {
          return _id;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief a cleanup function for the index. TODO: is this still necessary
/// the default implementation does nothing
////////////////////////////////////////////////////////////////////////////////

        virtual void cleanup ();

////////////////////////////////////////////////////////////////////////////////
/// @brief provide a size hint for the index. this is called when opening a
/// collection and the amount of documents in the collection is known. the 
/// purpose of this method is that the index can allocate enough memory to 
/// hold the number of documents specified and avoid later reallocations.
/// the default implementation does nothing
////////////////////////////////////////////////////////////////////////////////

        virtual void sizeHint (size_t);

////////////////////////////////////////////////////////////////////////////////
/// @brief insert a document into the index
/// this method is called only when opening a collection. it is guaranteed that
/// there will not be concurrent access to the index. implementations can use
/// this method to insert the document without acquiring locks
////////////////////////////////////////////////////////////////////////////////
        
        virtual void insert (struct TRI_doc_mptr_t*) = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief insert a document into the index
/// this method is called for regular document insertions. it is NOT guaranteed
/// that there isn't concurrent access to the index.
////////////////////////////////////////////////////////////////////////////////

        virtual void insert (TransactionCollection*,
                             Transaction*,
                             struct TRI_doc_mptr_t*) = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief remove a document from the index
/// this method is called for regular document removals. it is NOT guaranteed
/// that there isn't concurrent access to the index.
////////////////////////////////////////////////////////////////////////////////

        virtual struct TRI_doc_mptr_t* remove (TransactionCollection*,
                                               Transaction*,
                                               std::string const&,
                                               struct TRI_doc_mptr_t*) = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief forget a just-inserted document
/// this method is called when an insert or update method aborts and needs to
/// roll back changes made to the index. it is NOT guaranteed
/// that there isn't concurrent access to the index.
////////////////////////////////////////////////////////////////////////////////

        virtual void forget (TransactionCollection*,
                             Transaction*,
                             struct TRI_doc_mptr_t*) = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief method called directly before a commit is performed
////////////////////////////////////////////////////////////////////////////////

        virtual void preCommit (TransactionCollection*,
                                Transaction*) = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief return the amount of memory used by the index
////////////////////////////////////////////////////////////////////////////////

        virtual size_t memory () = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief return the index type
////////////////////////////////////////////////////////////////////////////////

        virtual IndexType type () const = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief return the index type name (e.g. "primary", "hash")
////////////////////////////////////////////////////////////////////////////////

        virtual std::string typeName () const = 0;
        
////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not the index can provide a selectivity estimate
////////////////////////////////////////////////////////////////////////////////
        
        virtual bool hasSelectivity () const = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief this method provides the selectivity estimate for the index. it 
/// should only be called for indexes whose hasSelectivity() method returned
/// `true`. A return value of 1.0 means highest selectivity, 0.0 means lowest
/// selectivity. Empty indexes may return 1.0.
////////////////////////////////////////////////////////////////////////////////

        virtual double getSelectivity () const = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief return index figures in JSON format
////////////////////////////////////////////////////////////////////////////////

        virtual triagens::basics::Json figures () const;

////////////////////////////////////////////////////////////////////////////////
/// @brief return a JSON representation of the index - this is used to persist
/// index definitions
////////////////////////////////////////////////////////////////////////////////

        virtual triagens::basics::Json toJson (TRI_memory_zone_t*) const;

////////////////////////////////////////////////////////////////////////////////
/// @brief "click" the index lock (if any)
////////////////////////////////////////////////////////////////////////////////

        virtual void clickLock () = 0;

////////////////////////////////////////////////////////////////////////////////
/// @brief return the index type based on a type name
////////////////////////////////////////////////////////////////////////////////

        static IndexType type (char const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief return the name of an index type
////////////////////////////////////////////////////////////////////////////////

        static char const* type (IndexType);

////////////////////////////////////////////////////////////////////////////////
/// @brief generate a new index id
////////////////////////////////////////////////////////////////////////////////

        static TRI_idx_iid_t generateId ();

////////////////////////////////////////////////////////////////////////////////
/// @brief validate an index id
////////////////////////////////////////////////////////////////////////////////

        static bool validateId (char const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief validate an index id (collection name + / + index id)
////////////////////////////////////////////////////////////////////////////////

        static bool validateId (char const*,
                                size_t*);

////////////////////////////////////////////////////////////////////////////////
/// @brief index comparator, used by the coordinator to detect if two index
/// contents are the same
////////////////////////////////////////////////////////////////////////////////

        static bool IndexComparator (struct TRI_json_t const*, 
                                     struct TRI_json_t const*);

// -----------------------------------------------------------------------------
// --SECTION--                                               protected variables
// -----------------------------------------------------------------------------

      protected:

////////////////////////////////////////////////////////////////////////////////
/// @brief the index id
////////////////////////////////////////////////////////////////////////////////

        TRI_idx_iid_t const               _id;

////////////////////////////////////////////////////////////////////////////////
/// @brief the index attribute names (may be empty if not used by an index)
////////////////////////////////////////////////////////////////////////////////

        std::vector<std::string> const    _fields;
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
