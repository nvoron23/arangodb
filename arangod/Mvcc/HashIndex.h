////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC hash index
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

#ifndef ARANGODB_MVCC_HASH_INDEX_H
#define ARANGODB_MVCC_HASH_INDEX_H 1

#include "Basics/Common.h"
#include "Basics/JsonHelper.h"
#include "Mvcc/Index.h"
#include "ShapedJson/shaped-json.h"

namespace triagens {
  namespace mvcc {

// -----------------------------------------------------------------------------
// --SECTION--                                                   class HashIndex
// -----------------------------------------------------------------------------

    class HashIndex : public Index {

// -----------------------------------------------------------------------------
// --SECTION--                                                      public types
// -----------------------------------------------------------------------------
      
      public:

        struct Element {
          TRI_doc_mptr_t*  _document;
          TRI_shaped_sub_t _subObjects[];
        };

        typedef std::vector<TRI_shaped_json_t> Key;

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------
      
      public:

        HashIndex (TRI_idx_iid_t,
                   struct TRI_document_collection_t*,
                   std::vector<std::string> const&,
                   std::vector<TRI_shape_pid_t> const&,
                   bool,
                   bool);

        ~HashIndex ();

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

        void cleanup () override final;

        void sizeHint (size_t) override final;
  
        bool hasSelectivity () const override final {
          return true;
        }

        double getSelectivity () const override final {
          return _theHash->selectivity();
        }

        size_t memory () override final;

        triagens::basics::Json toJson (TRI_memory_zone_t*) const override final;

        TRI_idx_type_e type () const override final {
          return TRI_IDX_TYPE_HASH_INDEX;
        }
        
        std::string typeName () const override final {
          return "hash";
        }
        
        void clickLock () override final {
          _lock.writeLock();
          _lock.writeUnlock();
        }

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

      public:
        
        size_t nrIndexedFields () const {
          return _paths.size();
        }

        std::vector<TRI_shape_pid_t> const& paths () const {
          return _paths;
        }
        
        std::vector<TRI_doc_mptr_t*>* lookup (TransactionCollection*,
                                              Transaction*,
                                              Key const*,
                                              size_t);

        std::vector<TRI_doc_mptr_t*>* lookupContinue (TransactionCollection*,
                                                      Transaction*,
                                                      TRI_doc_mptr_t*,
                                                      size_t);

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

      private:

        std::vector<TRI_doc_mptr_t*>* lookupInternal (TransactionCollection*,
                                                      Transaction*,
                                                      Key const*,
                                                      TRI_doc_mptr_t*,
                                                      size_t);

        size_t elementSize () const;

        Element* allocateAndFillElement (TRI_doc_mptr_t*,
                                         bool&);

        void deleteElement (Element*);

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief typedef for the hash table
////////////////////////////////////////////////////////////////////////////////

        typedef triagens::basics::AssocMulti<Key, Element, uint32_t> Hash_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief the index R/W lock
////////////////////////////////////////////////////////////////////////////////
        
        triagens::basics::ReadWriteLock   _lock;

////////////////////////////////////////////////////////////////////////////////
/// @brief the attribute paths
////////////////////////////////////////////////////////////////////////////////
        
        std::vector<TRI_shape_pid_t> const  _paths;

////////////////////////////////////////////////////////////////////////////////
/// @brief the hash table
////////////////////////////////////////////////////////////////////////////////

        Hash_t* _theHash;

////////////////////////////////////////////////////////////////////////////////
/// @brief whether the index is unique
////////////////////////////////////////////////////////////////////////////////

        bool _unique;

////////////////////////////////////////////////////////////////////////////////
/// @brief whether the index is sparse
////////////////////////////////////////////////////////////////////////////////

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
