////////////////////////////////////////////////////////////////////////////////
/// @brief associative array implementation tolerating repeated keys
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
/// @author Dr. Frank Celler
/// @author Martin Schoenert
/// @author Max Neunhoeffer
/// @author Copyright 2014, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2006-2014, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGODB_BASICS_ASSOC_MULTI_H
#define ARANGODB_BASICS_ASSOC_MULTI_H 1

//#if 0
// Activate for additional debugging: 
#define TRI_CHECK_MULTI_POINTER_HASH 1 
//#endif 

#include "Basics/Common.h"
#include "Basics/prime-numbers.h"

namespace triagens {
  namespace basics {

// -----------------------------------------------------------------------------
// --SECTION--                                        MULTI ASSOCIATIVE POINTERS
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                      public types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief associative array of pointers, tolerating repeated keys.
///
/// This is a data structure that can store pointers to elements. Each element
/// has a unique key (for example a certain attribute) and multiple
/// elements in the associative array can have the same key. Every element
/// can be at most once in the array.
/// We want to offer constant time complexity for the following
/// operations:
///  - insert pointer to a element into the array
///  - lookup pointer to a element in the array
///  - delete pointer to a element from the array
///  - find one pointer to a element with a given key
/// Furthermore, we want to offer O(n) complexity for the following
/// operation:
///  - find all pointers whose elements have a given key k, where n is
///    the number of elements in the array with this key
/// To this end, we use a hash table and ask the user to provide the following:
///  - a way to hash elements by their keys, and to hash keys themselves,
///  - a way to hash elements by their full identity
///  - a way to compare a key to the key of a given element
///  - a way to compare two elements, either by their keys or by their full
///    identities.
/// To avoid unnecessary comparisons the user can guarantee that s/he will
/// only try to store non-identical elements into the array. This enables
/// the code to skip comparisons which would otherwise be necessary to
/// ensure uniqueness.
/// The idea of the algorithm is as follows: Each slot in the hash table
/// contains a pointer to the actual element, as well as two unsigned
/// integers "prev" and "next" (being indices in the hash table) to
/// organise a linked list of entries, *within the same hash table*. All
/// elements with the same key are kept in a doubly linked list. The first
/// element in such a linked list is kept at the position determined by
/// its hash with respect to its key (or in the first free slot after this
/// position). All further elements in such a linked list are kept at the
/// position determined by its hash with respect to its full identity
/// (or in the first free slot after this position). Provided the hash
/// table is large enough and the hash functions distribute well enough,
/// this gives the proposed complexity.
///
////////////////////////////////////////////////////////////////////////////////

    template <class Key, class Element, class IndexType = size_t>
    class AssocMulti {

      public:
        static IndexType const INVALID_INDEX = ((IndexType)0)-1;

        typedef std::function<IndexType(Key const*)> HashKeyFuncType;
        typedef std::function<IndexType(Element const*, bool)> HashElementFuncType;
        typedef std::function<bool(Key const*, 
                                   Element const*)> IsEqualKeyElementFuncType;
        typedef std::function<bool(Element const*, 
                                   Element const*,
                                   bool byKey)> IsEqualElementElementFuncType;

      private:

        struct Entry {
          Element* ptr;      // a pointer to the data stored in this slot
          IndexType next;  // index of the data following in the linked
                           // list of all items with the same key
          IndexType prev;  // index of the data preceding in the linked
                           // list of all items with the same key
        };

        IndexType _nrAlloc;     // the size of the table
        IndexType _nrUsed;      // the number of used entries

        Entry* _table;         // the table itself

#ifdef TRI_INTERNAL_STATS
        uint64_t _nrFinds;   // statistics: number of lookup calls
        uint64_t _nrAdds;    // statistics: number of insert calls
        uint64_t _nrRems;    // statistics: number of remove calls
        uint64_t _nrResizes; // statistics: number of resizes

        uint64_t _nrProbes;  // statistics: number of misses in FindElementPlace
                             // and LookupByElement, used by insert, lookup and
                             // remove
        uint64_t _nrProbesF; // statistics: number of misses while looking up
        uint64_t _nrProbesD; // statistics: number of misses while removing
#endif

        HashKeyFuncType _hashKey;
        HashElementFuncType _hashElement;
        IsEqualKeyElementFuncType _isEqualKeyElement;
        IsEqualElementElementFuncType _isEqualElementElement;

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief constructor
////////////////////////////////////////////////////////////////////////////////

      public:

        AssocMulti (HashKeyFuncType hashKey,
                    HashElementFuncType hashElement,
                    IsEqualKeyElementFuncType isEqualKeyElement,
                    IsEqualElementElementFuncType isEqualElementElement,
                    IndexType initialSize = 64) 
          : _nrAlloc(initialSize),
            _nrUsed(0),
            _table(nullptr),
#ifdef TRI_INTERNAL_STATS
            _nrFinds(0), _nrAdds(0), _nrRems(0), _nrResizes(0),
            _nrProbes(0), _nrProbesF(0), _nrProbesD(0),
#endif
            _hashKey(hashKey), 
            _hashElement(hashElement),
            _isEqualKeyElement(isEqualKeyElement),
            _isEqualElementElement(isEqualElementElement) {

          try {
            _table = new Entry[_nrAlloc];
          }
          catch (...) {
            _table = nullptr;
            _nrAlloc = 0;
            throw;
          }
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief destructor
////////////////////////////////////////////////////////////////////////////////

        ~AssocMulti () {
          if (_table != nullptr) {
            delete [] _table;
            _table = nullptr;
          }
        }


// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief return the memory used by the hash table
////////////////////////////////////////////////////////////////////////////////

        size_t memoryUsage () const {
          return static_cast<size_t> (_nrAlloc * sizeof(Entry));
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief size(), return the number of items stored
////////////////////////////////////////////////////////////////////////////////

        size_t size () const {
          return static_cast<size_t>(_nrUsed);
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief capacity(), return the number of allocated items
////////////////////////////////////////////////////////////////////////////////

        size_t capacity () const {
          return static_cast<size_t>(_nrAlloc);
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief adds a key/element to the array
////////////////////////////////////////////////////////////////////////////////

        Element* insert (Element* element, 
                         bool const overwrite,
                         bool const checkEquality) {

          // if the checkEquality flag is not set, we do not check for element
          // equality we use this flag to speed up initial insertion into the
          // index, i.e. when the index is built for a collection and we know
          // for sure no duplicate elements will be inserted

          Element* old;

#ifdef TRI_CHECK_MULTI_POINTER_HASH
          check(true, true);
#endif

          // if we were adding and the table is more than half full, extend it
          if (_nrAlloc < 2 * _nrUsed) {
            resizeInternal(2 * _nrAlloc + 1);
          }

#ifdef TRI_INTERNAL_STATS
          // update statistics
          _nrAdds++;
#endif

          // compute the hash by the key only first
          IndexType hash = _hashElement(element, true);
          IndexType i = hash % _nrAlloc;

          // If this slot is free, just use it:
          if (nullptr == _table[i].ptr) {
            _table[i].ptr = element;
            _table[i].next = INVALID_INDEX;
            _table[i].prev = INVALID_INDEX;
            _nrUsed++;
#ifdef TRI_CHECK_MULTI_POINTER_HASH
            check(true, true);
#endif
            return nullptr;
          }

          // Now find the first slot with an entry with the same key
          // that is the start of a linked list, or a free slot:
          while (_table[i].ptr != nullptr &&
                 (! _isEqualElementElement(element, _table[i].ptr, true) ||
                  _table[i].prev != INVALID_INDEX)) {
            i = incr(i);
#ifdef TRI_INTERNAL_STATS
          // update statistics
            _ProbesA++;
#endif

          }

          // If this is free, we are the first with this key:
          if (nullptr == _table[i].ptr) {
            _table[i].ptr = element;
            _table[i].next = INVALID_INDEX;
            _table[i].prev = INVALID_INDEX;
            _nrUsed++;
#ifdef TRI_CHECK_MULTI_POINTER_HASH
            check(true, true);
#endif
            return nullptr;
          }

          // Otherwise, entry i points to the beginning of the linked
          // list of which we want to make element a member. Perhaps an
          // equal element is right here:
          if (checkEquality && 
              _isEqualElementElement(element, _table[i].ptr, false)) {
            old = _table[i].ptr;
            if (overwrite) {
              _table[i].ptr = element;
            }
#ifdef TRI_CHECK_MULTI_POINTER_HASH
            check(true, true);
#endif
            return old;
          }

          // Now find a new home for element in this linked list:
          IndexType j = findElementPlace(element, checkEquality);

          old = _table[j].ptr;

          // if we found an element, return
          if (old != nullptr) {
            if (overwrite) {
              _table[j].ptr = element;
            }
#ifdef TRI_CHECK_MULTI_POINTER_HASH
            check(true, true);
#endif
            return old;
          }

          // add a new element to the associative array and linked list (in pos 2):
          _table[j].ptr = element;
          _table[j].next = _table[i].next;
          _table[j].prev = i;
          _table[i].next = j;
          // Finally, we need to find the successor to patch it up:
          if (_table[j].next != INVALID_INDEX) {
            _table[_table[j].next].prev = j;
          }
          _nrUsed++;

#ifdef TRI_CHECK_MULTI_POINTER_HASH
          check(true, true);
#endif
          return nullptr;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief lookups an element given an element
////////////////////////////////////////////////////////////////////////////////

        Element* lookup (Element const* element) const {
          IndexType i;

#ifdef TRI_INTERNAL_STATS
          // update statistics
          _nrFinds++;
#endif

          i = lookupByElement(element);
          return _table[i].ptr;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief lookups an element given a key
////////////////////////////////////////////////////////////////////////////////

        std::vector<Element*>* lookupByKey (Key const* key) const {
          auto result = new std::vector<Element*>();

          // compute the hash
          IndexType hash = _hashKey(key);
          IndexType i = hash % _nrAlloc;

#ifdef TRI_INTERNAL_STATS
          // update statistics
          _nrFinds++;
#endif

          // search the table
          while (_table[i].ptr != nullptr &&
                 (! _isEqualKeyElement(key, _table[i].ptr) ||
                  _table[i].prev != INVALID_INDEX)) {
            i = incr(i);
#ifdef TRI_INTERNAL_STATS
            _nrProbesF++;
#endif
          }

          if (_table[i].ptr != nullptr) {
            // We found the beginning of the linked list:

            // pre-initialize the result to save at least a few reallocs
            result->reserve(4);
            do {
              result->push_back(_table[i].ptr);
              i = _table[i].next;
            } 
            while (i != INVALID_INDEX);
          }

          // return whatever we found
          return result;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief removes an element from the array
////////////////////////////////////////////////////////////////////////////////

        Element* remove (Element const* element) {
          IndexType j = 0;

#ifdef TRI_INTERNAL_STATS
          // update statistics
          _nrRems++;
#endif

#ifdef TRI_CHECK_MULTI_POINTER_HASH
          check(true, true);
#endif
          IndexType i = lookupByElement(element);
          if (_table[i].ptr == nullptr) {
            return nullptr;
          }

          Element* old = _table[i].ptr;
          // We have to delete entry i
          if (_table[i].prev == INVALID_INDEX) {
            // This is the first in its linked list.
            j = _table[i].next;
            if (j == INVALID_INDEX) {
              // The only one in its linked list, simply remove it and heal
              // the hole:
              invalidateEntry(i);
#ifdef TRI_CHECK_MULTI_POINTER_HASH
              check(false, false);
#endif
              healHole(i);
            }
            else {
              // There is at least one successor in position j.
              _table[j].prev = INVALID_INDEX;
              moveEntry(j, i);
#ifdef TRI_CHECK_MULTI_POINTER_HASH
              check(false, false);
#endif
              healHole(j);
            }
          }
          else {
            // This one is not the first in its linked list
            j = _table[i].prev;
            _table[j].next = _table[i].next;
            j = _table[i].next;
            if (j != INVALID_INDEX) {
              // We are not the last in the linked list.
              _table[j].prev = _table[i].prev;
            }
            invalidateEntry(i);
#ifdef TRI_CHECK_MULTI_POINTER_HASH
            check(false, false);
#endif
            healHole(i);
          }
          _nrUsed--;
#ifdef TRI_CHECK_MULTI_POINTER_HASH
          check(true, true);
#endif
          // return success
          return old;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief resize the array
////////////////////////////////////////////////////////////////////////////////

        int resize (IndexType size) throw() {
          if (2*size+1 < _nrUsed) {
            return TRI_ERROR_BAD_PARAMETER;
          }

          try {
            resizeInternal(2*size+1);
          }
          catch (...) {
            return TRI_ERROR_OUT_OF_MEMORY;
          }
          return TRI_ERROR_NO_ERROR;
        }

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief increment IndexType by 1 modulo _nrAlloc:
////////////////////////////////////////////////////////////////////////////////

        inline IndexType incr (IndexType i) const {
          IndexType dummy = (++i) - _nrAlloc;
          return i < _nrAlloc ? i : dummy;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief resize the array, internal method
////////////////////////////////////////////////////////////////////////////////

        void resizeInternal (IndexType size) {
          Entry* oldTable = _table;
          IndexType oldAlloc = _nrAlloc;

          _nrAlloc = TRI_NearPrime(size);
          try {
            _table = new Entry[_nrAlloc];
          }
          catch (...) {
            _nrAlloc = oldAlloc;
            _table = oldTable;
            throw;
          }

          _nrUsed = 0;
#ifdef TRI_INTERNAL_STATS
          _nrResizes++;
#endif

          // table is already clear by allocate, copy old data
          IndexType j;
          for (j = 0; j < oldAlloc; j++) {
            if (oldTable[j].ptr != nullptr) {
              insert(oldTable[j].ptr, true, false);
            }
          }

          delete [] oldTable;
        }

#ifdef TRI_CHECK_MULTI_POINTER_HASH

////////////////////////////////////////////////////////////////////////////////
/// @brief internal debugging check function
////////////////////////////////////////////////////////////////////////////////

        bool check (bool checkCount, bool checkPositions) const {
          IndexType i, ii, j, k;

          bool ok = true;
          IndexType count = 0;

          for (i = 0;i < _nrAlloc;i++) {
            if (_table[i].ptr != nullptr) {
              count++;
              if (_table[i].prev != INVALID_INDEX) {
                if (_table[_table[i].prev].next != i) {
                  std::cout << "Alarm prev " << i << std::endl;
                  ok = false;
                }
              }
              if (_table[i].next != INVALID_INDEX) {
                if (_table[_table[i].next].prev != i) {
                  std::cout << "Alarm next " << i << std::endl;
                  ok = false;
                }
              }
              ii = i;
              j = _table[ii].next;
              while (j != INVALID_INDEX) {
                if (j == i) {
                  std::cout << "Alarm cycle " << i << std::endl;
                  ok = false;
                  break;
                }
                ii = j;
                j = _table[ii].next;
              }
            }
          }
          if (checkCount && count != _nrUsed) {
            std::cout << "Alarm _nrUsed wrong " << _nrUsed << " != "
                      << count << "!" << std::endl;
            ok = false;
          }
          if (checkPositions) {
            for (i = 0;i < _nrAlloc;i++) {
              if (_table[i].ptr != nullptr) {
                IndexType hash;
                if (_table[i].prev == INVALID_INDEX) {
                  // We are the first in a linked list.
                  hash = _hashElement(_table[i].ptr, true);
                  j = hash % _nrAlloc;
                  for (k = j; k != i; ) {
                    if (_table[k].ptr == nullptr ||
                        (_table[k].prev == INVALID_INDEX &&
                         _isEqualElementElement(_table[i].ptr,
                                                _table[k].ptr,
                                                true))) {
                      ok = false;
                      std::cout << "Alarm pos bykey: " << i << std::endl;
                    }
                    k = incr(k);
                  }
                }
                else {
                  // We are not the first in a linked list.
                  hash = _hashElement(_table[i].ptr, false);
                  j = hash % _nrAlloc;
                  for (k = j; k != i; ) {
                    if (_table[k].ptr == nullptr ||
                        _isEqualElementElement(_table[i].ptr,
                                               _table[k].ptr, false)) {
                      ok = false;
                      std::cout << "Alarm unique: " << k << ", " 
                                << i << std::endl;
                    }
                    k = incr(k);
                  }
                }
              }
            }
          }
          if (! ok) {
            std::cout << "Something is wrong!" << std::endl;
          }
          return ok;
        }

#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief find an element or its place using the element hash function
////////////////////////////////////////////////////////////////////////////////

        inline IndexType findElementPlace (Element const* element,
                                           bool checkEquality) const {

          // This either finds a place to store element or an entry in
          // the table that is equal to element. If checkEquality is
          // set to false, the caller guarantees that there is no entry
          // that compares equal to element in the table, which saves a
          // lot of element comparisons. This function always returns a
          // pointer into the table, which is either empty or points to
          // an entry that compares equal to element.

          IndexType hash = _hashElement(element, false);
          IndexType i = hash % _nrAlloc;

          while (_table[i].ptr != nullptr &&
                 (! checkEquality ||
                  ! _isEqualElementElement(element, _table[i].ptr, false))) {
            i = incr(i);
#ifdef TRI_INTERNAL_STATS
            _nrProbes++;
#endif
          }
          return i;
        }
 
////////////////////////////////////////////////////////////////////////////////
/// @brief find an element or its place by key or element identity
////////////////////////////////////////////////////////////////////////////////

        IndexType lookupByElement (Element const* element) const {
          // This performs a complete lookup for an element. It returns a slot
          // number. This slot is either empty or contains an element that
          // compares equal to element.
          //
          IndexType hash = _hashElement(element, true);
          IndexType i = hash % _nrAlloc;

          // Now find the first slot with an entry with the same key
          // that is the start of a linked list, or a free slot:
          while (_table[i].ptr != nullptr &&
                 (_table[i].prev != INVALID_INDEX ||
                  ! _isEqualElementElement(element, _table[i].ptr, true))) {
            i = incr(i);
#ifdef TRI_INTERNAL_STATS
            _nrProbes++;
#endif
          }

          if (_table[i].ptr != nullptr) {
            // It might be right here!
            if (_isEqualElementElement(element, _table[i].ptr, false)) {
              return i;
            }

            // Now we have to look for it in its hash position:
            IndexType j = findElementPlace(element, true);

            // We have either found an equal element or nothing:
            return j;
          }

          // If we get here, no element with the same key is in the array, so
          // we will not be able to find it anywhere!
          return i;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief helper to decide whether something is between to places
////////////////////////////////////////////////////////////////////////////////

        static inline bool isBetween (IndexType from,
                                      IndexType x,
                                      IndexType to) {
          // returns whether or not x is behind from and before or equal to
          // to in the cyclic order. If x is equal to from, then the result is
          // always false. If from is equal to to, then the result is always
          // true.
          return (from < to) ? (from < x && x <= to)
                             : (x > from || x <= to);
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief helper to invalidate a slot
////////////////////////////////////////////////////////////////////////////////

        inline void invalidateEntry (IndexType i) {
          _table[i].ptr = nullptr;
          _table[i].next = INVALID_INDEX;
          _table[i].prev = INVALID_INDEX;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief helper to move an entry from one slot to another
////////////////////////////////////////////////////////////////////////////////

        inline void moveEntry (IndexType from, IndexType to) {
          // Moves an entry, adjusts the linked lists, but does not take care
          // for the hole. to must be unused. from can be any element in a
          // linked list.
          _table[to] = _table[from];
          if (_table[to].prev != INVALID_INDEX) {
            _table[_table[to].prev].next = to;
          }
          if (_table[to].next != INVALID_INDEX) {
            _table[_table[to].next].prev = to;
          }
          invalidateEntry(from);
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief helper to heal a hole where we deleted something
////////////////////////////////////////////////////////////////////////////////

        void healHole (IndexType i) {
          IndexType j = incr(i);

          while (_table[j].ptr != nullptr) {
            // Find out where this element ought to be:
            // If it is the start of one of the linked lists, we need to hash
            // by key, otherwise, we hash by the full identity of the element:
            IndexType hash = _hashElement(_table[j].ptr,
                                          _table[j].prev == INVALID_INDEX);
            IndexType k = hash % _nrAlloc;
            if (! isBetween(i, k, j)) {
              // we have to move j to i:
              moveEntry(j, i);
              i = j;  // Now heal this hole at j, 
                      // j will be incremented right away
            }
            j = incr(j);
#ifdef TRI_INTERNAL_STATS
            _nrProbesD++;
#endif
          }
        }

    };

  }  // namespace triagens::basics
} // namespace triagens

#endif

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
