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
#include "Basics/ReadWriteLock.h"
#include "Basics/SkipList.h"
#include "Mvcc/Index.h"
#include "ShapedJson/shaped-json.h"

namespace triagens {
  namespace mvcc {

// -----------------------------------------------------------------------------
// --SECTION--                                               class SkiplistIndex
// -----------------------------------------------------------------------------

    class SkiplistIndex2 : public Index {

// -----------------------------------------------------------------------------
// --SECTION--                                                      public types
// -----------------------------------------------------------------------------
      
      public:

        struct Element {
          TRI_doc_mptr_t*  _document;
          TRI_shaped_sub_t _subObjects[];
        };

        typedef std::vector<TRI_shaped_json_t> Key;

////////////////////////////////////////////////////////////////////////////////
/// @brief typedef for the skiplist
////////////////////////////////////////////////////////////////////////////////
        
        typedef triagens::basics::SkipList<Key, Element> SkipList_t;

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------
      
      public:

        SkiplistIndex2 (TRI_idx_iid_t,
                        struct TRI_document_collection_t*,
                        std::vector<std::string> const&,
                        std::vector<TRI_shape_pid_t> const&,
                        bool,
                        bool);

        ~SkiplistIndex2 ();

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
        
        void clickLock () override final {
          _lock.writeLock();
          _lock.writeUnlock();
        }

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

      public:
        
////////////////////////////////////////////////////////////////////////////////
/// @brief Iterator structure for skip list. We require a start and stop node
///
/// Intervals are open in the sense that both end points are not members
/// of the interval. This means that one has to use SkipList_t::nextNode
/// on the start node to get the first element and that the stop node
/// can be NULL. Note that it is ensured that all intervals in an iterator
/// are non-empty.
////////////////////////////////////////////////////////////////////////////////

        struct Interval {
          SkipList_t::Node* _leftEndPoint;
          SkipList_t::Node* _rightEndPoint;

          Interval ()
            : _leftEndPoint(nullptr), _rightEndPoint(nullptr) {
          }

          ~Interval () {
          }
        };

        class Iterator {
            SkiplistIndex2 const* _index;
            TransactionCollection* _collection;
            Transaction* _transaction;
            bool _reverse;
            std::vector<Interval> _intervals;
            size_t _currentInterval; // starts with 0, current interval used
            SkipList_t::Node* _cursor;
                     // always holds the last node returned, initially equal to
                     // the _leftEndPoint of the first interval (or the 
                     // _rightEndPoint of the last interval in the reverse
                     // case), can be nullptr if there are no intervals
                     // (yet), or, in the reverse case, if the cursor is
                     // at the end of the last interval. Additionally
                     // in the non-reverse case _cursor is set to nullptr
                     // if the cursor is exhausted.
                     // See SkiplistNextIterationCallback and
                     // SkiplistPrevIterationCallback for the exact
                     // condition for the iterator to be exhausted.
          public:
            Iterator (SkiplistIndex2 const* index, TransactionCollection* coll,
                      Transaction* trans, TRI_index_operator_t const* op,
                      bool reverse)
              : _index(index), _collection(coll), _transaction(trans),
                _reverse(reverse), _currentInterval(0), _cursor(nullptr) {
              fillMe(op);
            }
            ~Iterator () {
            }
            // Note due to MVCC hasNext is allowed to return true and
            // a subsequent call to next() still returns nullptr. This is
            // because only next() actually invokes the MVCC logic to check
            // visibility.
            bool hasNext () const;
            Element* next ();
            void skip (size_t);
          private:
            void fillMe (TRI_index_operator_t const* op);
            void fillHelper (TRI_index_operator_t const* op,
                             std::vector<Interval>& result);
            bool intervalIntersectionValid(Interval const& left,
                                           Interval const& right,
                                           Interval& result);
            bool intervalValid (Interval const& interval); 
            Interval getInterval () const;
        };

        size_t nrIndexedFields () const {
          return _paths.size();
        }

        std::vector<TRI_shape_pid_t> const& paths () const {
          return _paths;
        }

        Iterator* lookup (TransactionCollection*,
                          Transaction*,
                          struct TRI_index_operator_s*,
                          bool) const;
        
        void deleteElement (Element*);

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------
  
      private:

        size_t elementSize () const;

        Element* allocateAndFillElement (TRI_doc_mptr_t*,
                                         bool&);

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief the index R/W lock
////////////////////////////////////////////////////////////////////////////////
        
        triagens::basics::ReadWriteLock   _lock;

////////////////////////////////////////////////////////////////////////////////
/// @brief the attribute paths
////////////////////////////////////////////////////////////////////////////////

        std::vector<TRI_shape_pid_t> const  _paths;

////////////////////////////////////////////////////////////////////////////////
/// @brief the skiplist
////////////////////////////////////////////////////////////////////////////////

        SkipList_t* _theSkipList;

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
