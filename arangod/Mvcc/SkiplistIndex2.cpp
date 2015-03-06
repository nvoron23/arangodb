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
/// @author Max Neunhoeffer
/// @author Copyright 2015, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "SkiplistIndex2.h"
#include "Basics/ReadLocker.h"
#include "Basics/WriteLocker.h"
#include "Mvcc/Transaction.h"
#include "Mvcc/TransactionCollection.h"
#include "Utils/Exception.h"
#include "VocBase/document-collection.h"
#include "VocBase/voc-shaper.h"

using namespace triagens::basics;
using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                               class SkiplistIndex
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                   private helpers
// -----------------------------------------------------------------------------

// .............................................................................
// recall for all of the following comparison functions:
//
// left < right  return -1
// left > right  return  1
// left == right return  0
//
// furthermore:
//
// the following order is currently defined for placing an order on documents
// undef < null < boolean < number < strings < lists < hash arrays
// note: undefined will be treated as NULL pointer not NULL JSON OBJECT
// within each type class we have the following order
// boolean: false < true
// number: natural order
// strings: lexicographical
// lists: lexicographically and within each slot according to these rules.
// ...........................................................................

////////////////////////////////////////////////////////////////////////////////
/// @brief compares a key with an element, version with proper types
////////////////////////////////////////////////////////////////////////////////

static int CompareKeyElement (TRI_shaped_json_t const* left,
                              SkiplistIndex2::Element const* right,
                              size_t rightPosition,
                              TRI_shaper_t* shaper) {
  TRI_ASSERT(nullptr != left);
  TRI_ASSERT(nullptr != right);

  int result = TRI_CompareShapeTypes(nullptr,
                                     nullptr,
                                     left,
                                     shaper,
                                     right->_document->getShapedJsonPtr(),
                                     &right->_subObjects[rightPosition],
                                     nullptr,
                                     shaper);

  // ...........................................................................
  // In the above function CompareShapeTypes we use strcmp which may
  // return an integer greater than 1 or less than -1. From this
  // function we only need to know whether we have equality (0), less
  // than (-1) or greater than (1)
  // ...........................................................................

  if (result < 0) {
    result = -1;
  }
  else if (result > 0) {
    result = 1;
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief compares elements, version with proper types
////////////////////////////////////////////////////////////////////////////////

static int CompareElementElement (SkiplistIndex2::Element const* left,
                                  size_t leftPosition,
                                  SkiplistIndex2::Element const* right,
                                  size_t rightPosition,
                                  TRI_shaper_t* shaper) {
  TRI_ASSERT(nullptr != left);
  TRI_ASSERT(nullptr != right);

  int result = TRI_CompareShapeTypes(left->_document->getShapedJsonPtr(),
                                     &left->_subObjects[leftPosition],
                                     nullptr,
                                     shaper,
                                     right->_document->getShapedJsonPtr(),
                                     &right->_subObjects[rightPosition],
                                     nullptr,
                                     shaper);

  // ...........................................................................
  // In the above function CompareShapeTypes we use strcmp which may
  // return an integer greater than 1 or less than -1. From this
  // function we only need to know whether we have equality (0), less
  // than (-1) or greater than (1)
  // ...........................................................................

  if (result < 0) {
    return -1;
  }
  else if (result > 0) {
    return 1;
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief compares two elements in a skip list, this is the generic callback
////////////////////////////////////////////////////////////////////////////////

class CmpElmElm {
    TRI_shaper_t* _shaper;
    size_t _numFields;

  public:
    CmpElmElm (TRI_shaper_t* shaper, size_t numFields) 
      : _shaper(shaper), _numFields(numFields) {
    }
    ~CmpElmElm () {
    }
    int operator() (SkiplistIndex2::Element const* leftElement, 
                    SkiplistIndex2::Element const* rightElement, 
                    SkiplistIndex2::SkipList_t::CmpType cmptype) {
      TRI_ASSERT(nullptr != leftElement);
      TRI_ASSERT(nullptr != rightElement);

      // .......................................................................
      // The document could be the same -- so no further comparison is required.
      // .......................................................................

      if (leftElement == rightElement ||
          leftElement->_document == rightElement->_document) {
        return 0;
      }

      for (size_t j = 0;  j < _numFields;  j++) {
        int compareResult = CompareElementElement(leftElement,
                                                  j,
                                                  rightElement,
                                                  j,
                                                  _shaper);

        if (compareResult != 0) {
          return compareResult;
        }
      }

      // .......................................................................
      // This is where the difference between the preorder and the proper total
      // order comes into play. Here if the 'keys' are the same,
      // but the doc ptr is different (which it is since we are here), then
      // we return 0 if we use the preorder and look at the _key attribute
      // otherwise.
      // .......................................................................

      if (SkiplistIndex2::SkipList_t::CMP_PREORDER == cmptype) {
        return 0;
      }

      // We break this tie in the key comparison by first looking at the key...
      int compareResult = strcmp(TRI_EXTRACT_MARKER_KEY(leftElement->_document),
                                 TRI_EXTRACT_MARKER_KEY(rightElement->_document));

      if (compareResult < 0) {
        return -1;
      }
      else if (compareResult > 0) {
        return 1;
      }
      // ... and then at the revision:
      if (leftElement->_document->_rid < rightElement->_document->_rid) {
        return -1;
      }
      else if (leftElement->_document->_rid > rightElement->_document->_rid) {
        return 1;
      }
      return 0;
    }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief compares a key with an element in a skip list, generic callback
////////////////////////////////////////////////////////////////////////////////

class CmpKeyElm {
    TRI_shaper_t* _shaper;
    size_t _numFields;
  public:
    CmpKeyElm (TRI_shaper_t* shaper, size_t numFields) 
      : _shaper(shaper), _numFields(numFields) {
    }
    ~CmpKeyElm () {
    }
    int operator() (SkiplistIndex2::Key const*     leftKey, 
                    SkiplistIndex2::Element const* rightElement) {
      TRI_ASSERT(nullptr != leftKey);
      TRI_ASSERT(nullptr != rightElement);

      // Note that the key might contain fewer fields than there are indexed
      // attributes, therefore we only run the following loop to
      // leftKey->_numFields.
      for (size_t j = 0;  j < leftKey->size();  j++) {
        int compareResult = CompareKeyElement(&leftKey->at(j), 
                                              rightElement, j, _shaper);

        if (compareResult != 0) {
          return compareResult;
        }
      }

      return 0;
    }
};

////////////////////////////////////////////////////////////////////////////////
/// @brief frees an element in the skiplist
////////////////////////////////////////////////////////////////////////////////

class FreeElm {
    SkiplistIndex2* _skipListIndex;
  public:
    FreeElm (SkiplistIndex2* skipListIndex)
      : _skipListIndex(skipListIndex) {
    }
    ~FreeElm () {
    }
    void operator() (SkiplistIndex2::Element* elm) {
      _skipListIndex->deleteElement(elm);
    }
};

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new skiplist index
////////////////////////////////////////////////////////////////////////////////

SkiplistIndex2::SkiplistIndex2 (TRI_idx_iid_t id,
                                TRI_document_collection_t* collection,
                                std::vector<std::string> const& fields,
                                std::vector<TRI_shape_pid_t> const& paths,
                                bool unique,
                                bool sparse)
  : Index(id, fields),
    _collection(collection),
    _lock(),
    _paths(paths),
    _theSkipList(nullptr),
    _unique(unique),
    _sparse(sparse) {
  
  for (auto it : paths) {
    TRI_ASSERT(it != 0);
  }
  
  try {
    _theSkipList = new SkipList_t(CmpElmElm(collection->getShaper(),
                                            paths.size()),
                                  CmpKeyElm(collection->getShaper(),
                                            paths.size()),
                                  FreeElm(this),
                                  unique);
  }
  catch (...) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the skiplist index
////////////////////////////////////////////////////////////////////////////////

SkiplistIndex2::~SkiplistIndex2 () {
  delete _theSkipList;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------
        
////////////////////////////////////////////////////////////////////////////////
/// @brief lookup method, constructing an iterator
////////////////////////////////////////////////////////////////////////////////

SkiplistIndex2::Iterator* SkiplistIndex2::lookup (
                                  TransactionCollection* coll,
                                  Transaction* trans,
                                  TRI_index_operator_t* op,
                                  bool reverse) const {
  return new Iterator(this, coll, trans, op, reverse);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief Helper function for fillMe
////////////////////////////////////////////////////////////////////////////////

static int FillLookupSLOperator (TRI_index_operator_t* slOperator,
                                 TRI_document_collection_t* document) {
  if (slOperator == nullptr) {
    return TRI_ERROR_INTERNAL;
  }

  switch (slOperator->_type) {
    case TRI_AND_INDEX_OPERATOR:
    case TRI_NOT_INDEX_OPERATOR:
    case TRI_OR_INDEX_OPERATOR: {
      TRI_logical_index_operator_t* logicalOperator = (TRI_logical_index_operator_t*) slOperator;
      int result = FillLookupSLOperator(logicalOperator->_left, document);

      if (result == TRI_ERROR_NO_ERROR) {
        result = FillLookupSLOperator(logicalOperator->_right, document);
      }
      if (result != TRI_ERROR_NO_ERROR) {
        return result;
      }
      break;
    }

    case TRI_EQ_INDEX_OPERATOR:
    case TRI_GE_INDEX_OPERATOR:
    case TRI_GT_INDEX_OPERATOR:
    case TRI_NE_INDEX_OPERATOR:
    case TRI_LE_INDEX_OPERATOR:
    case TRI_LT_INDEX_OPERATOR: {
      TRI_relation_index_operator_t* relationOperator = (TRI_relation_index_operator_t*) slOperator;
      relationOperator->_numFields = relationOperator->_parameters->_value._objects._length;
      relationOperator->_fields = static_cast<TRI_shaped_json_t*>(TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, sizeof(TRI_shaped_json_t) * relationOperator->_numFields, false));

      if (relationOperator->_fields != nullptr) {
        for (size_t j = 0; j < relationOperator->_numFields; ++j) {
          TRI_json_t const* jsonObject = static_cast<TRI_json_t* const>(TRI_AtVector(&(relationOperator->_parameters->_value._objects), j));

          // find out if the search value is a list or an array
          if ((TRI_IsArrayJson(jsonObject) || TRI_IsObjectJson(jsonObject)) &&
              slOperator->_type != TRI_EQ_INDEX_OPERATOR) {
            // non-equality operator used on list or array data type, this is disallowed
            // because we need to shape these objects first. however, at this place (index lookup)
            // we never want to create new shapes so we will have a problem if we cannot find an
            // existing shape for the search value. in this case we would need to raise an error
            // but then the query results would depend on the state of the shaper and if it had
            // seen previous such objects

            // we still allow looking for list or array values using equality. this is safe.
            TRI_Free(TRI_UNKNOWN_MEM_ZONE, relationOperator->_fields);
            relationOperator->_fields = nullptr;
            return TRI_ERROR_BAD_PARAMETER;
          }

          // now shape the search object (but never create any new shapes)
          TRI_shaped_json_t* shapedObject = TRI_ShapedJsonJson(document->getShaper(), jsonObject, false);  // ONLY IN INDEX, PROTECTED by RUNTIME

          if (shapedObject != nullptr) {
            // found existing shape
            relationOperator->_fields[j] = *shapedObject; // shallow copy here is ok
            TRI_Free(TRI_UNKNOWN_MEM_ZONE, shapedObject); // don't require storage anymore
          }
          else {
            // shape not found
            TRI_Free(TRI_UNKNOWN_MEM_ZONE, relationOperator->_fields);
            relationOperator->_fields = nullptr;
            return TRI_RESULT_ELEMENT_NOT_FOUND;
          }
        }
      }
      else {
        relationOperator->_numFields = 0; // out of memory?
      }
      break;
    }
  }

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief fillMe, this actually builds up the iterator object
////////////////////////////////////////////////////////////////////////////////

void SkiplistIndex2::Iterator::fillMe (TRI_index_operator_t* op) {

  int errorResult = FillLookupSLOperator(op, _collection->documentCollection());
  if (errorResult != TRI_ERROR_NO_ERROR) {
    THROW_ARANGO_EXCEPTION(errorResult);
  }

  // Now actually fill ourselves:
  fillHelper(op, _intervals);

  size_t const n = _intervals.size();

  // Finally initialise _cursor if the result is not empty:
  if (0 < n) {
    if (_reverse) {
      // start at last interval, right endpoint
      _currentInterval = n - 1;
      _cursor = _intervals[n-1]._rightEndPoint;
    }
    else {
      // start at first interval, left endpoint
      _currentInterval = 0;
      _cursor = _intervals[0]._leftEndPoint;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief fillHelper, this actually builds up the iterator object, possibly
/// calling itself recursively
////////////////////////////////////////////////////////////////////////////////

void SkiplistIndex2::Iterator::fillHelper (TRI_index_operator_t* op,
                                           std::vector<Interval>& result) {
  Key                               values;
  std::vector<Interval>             leftResult;
  std::vector<Interval>             rightResult;
  TRI_relation_index_operator_t*    relationOperator;
  TRI_logical_index_operator_t*     logicalOperator;
  Interval                          interval;
  SkipList_t::Node*                 temp;
  SkipList_t::Node*                 prev;

  relationOperator  = (TRI_relation_index_operator_t*) op;
  logicalOperator   = (TRI_logical_index_operator_t*) op;

  switch (op->_type) {
    case TRI_EQ_INDEX_OPERATOR:
    case TRI_LE_INDEX_OPERATOR:
    case TRI_LT_INDEX_OPERATOR:
    case TRI_GE_INDEX_OPERATOR:
    case TRI_GT_INDEX_OPERATOR:
      for (size_t i = 0; i < relationOperator->_numFields; i++) {
        values.push_back(relationOperator->_fields[i]);
      }
      break;   // this is to silence a compiler warning
    default: {
      // must not access relationOperator->xxx if the operator is not a
      // relational one otherwise we'll get invalid reads and the prog
      // might crash
    }
  }

  switch (op->_type) {
    case TRI_AND_INDEX_OPERATOR: {
      fillHelper(logicalOperator->_left, leftResult);
      fillHelper(logicalOperator->_right, rightResult);

      for (size_t i = 0; i < leftResult.size(); ++i) {
        for (size_t j = 0; j < rightResult.size(); ++j) {
          if (intervalIntersectionValid(leftResult[i], rightResult[j],
                                        interval)) {
            result.push_back(interval);
          }
        }
      }
      return;
    }

    case TRI_EQ_INDEX_OPERATOR: {
      temp = _index->_theSkipList->leftKeyLookup(&values);
      TRI_ASSERT(nullptr != temp);
      if (_index->_unique) {
        // At most one hit, but we have to find the exact revision:
        CmpKeyElm comparer(_collection->shaper(), _index->nrIndexedFields());
        while (true) {  // will be left by break
          prev = temp;
          temp = temp->nextNode();
          if (nullptr == temp) {
            break;
          }
          triagens::mvcc::TransactionId from 
              = temp->document()->_document->from();
          triagens::mvcc::TransactionId to 
              = temp->document()->_document->to();

          if (_transaction->isVisibleForRead(from, to)) {
            if (0 == comparer(&values, temp->document())) {
              interval._leftEndPoint = prev;
              interval._rightEndPoint = temp->nextNode();
              if (intervalValid(interval)) {
                result.push_back(interval);
              }
            }
          }
        }
      }
      else {
        // Note that in this case (_unique == false) we return the interval
        // of all matching documents and all available revisions!
        interval._leftEndPoint = temp;
        temp = _index->_theSkipList->rightKeyLookup(&values);
        interval._rightEndPoint = temp->nextNode();
        if (intervalValid(interval)) {
          result.push_back(interval);
        }
      }
      return;
    }

    case TRI_LE_INDEX_OPERATOR: {
      interval._leftEndPoint  = _index->_theSkipList->startNode();
      temp = _index->_theSkipList->rightKeyLookup(&values);
      interval._rightEndPoint = temp->nextNode();

      if (intervalValid(interval)) {
        result.push_back(interval);
      }
      return;
    }

    case TRI_LT_INDEX_OPERATOR: {
      interval._leftEndPoint  = _index->_theSkipList->startNode();
      temp = _index->_theSkipList->leftKeyLookup(&values);
      interval._rightEndPoint = temp->nextNode();

      if (intervalValid(interval)) {
        result.push_back(interval);
      }
      return;
    }

    case TRI_GE_INDEX_OPERATOR: {
      temp = _index->_theSkipList->leftKeyLookup(&values);
      interval._leftEndPoint = temp;
      interval._rightEndPoint = _index->_theSkipList->endNode();

      if (intervalValid(interval)) {
        result.push_back(interval);
      }
      return;
    }

    case TRI_GT_INDEX_OPERATOR: {
      temp = _index->_theSkipList->rightKeyLookup(&values);
      interval._leftEndPoint = temp;
      interval._rightEndPoint = _index->_theSkipList->endNode();

      if (intervalValid(interval)) {
        result.push_back(interval);
      }
      return;
    }

    default: {
      TRI_ASSERT(false);
    }

  } // end of switch statement
}

////////////////////////////////////////////////////////////////////////////////
/// @brief intervalValid
////////////////////////////////////////////////////////////////////////////////

// .............................................................................
// Tests whether the LeftEndPoint is < than RightEndPoint (-1)
// Tests whether the LeftEndPoint is == to RightEndPoint (0)    [empty]
// Tests whether the LeftEndPoint is > than RightEndPoint (1)   [undefined]
// .............................................................................

bool SkiplistIndex2::Iterator::intervalValid ( Interval const& interval) {
  int compareResult;
  SkipList_t::Node* lNode;
  SkipList_t::Node* rNode;

  lNode = interval._leftEndPoint;

  if (lNode == nullptr) {
    return false;
  }
  // Note that the right end point can be nullptr to indicate the end of
  // the index.

  rNode = interval._rightEndPoint;

  if (lNode == rNode) {
    return false;
  }

  if (lNode->nextNode() == rNode) {
    // Interval empty, nothing to do with it.
    return false;
  }

  if (nullptr != rNode && rNode->nextNode() == lNode) {
    // Interval empty, nothing to do with it.
    return false;
  }

  if (_index->_theSkipList->getNrUsed() == 0) {
    return false;
  }

  if ( lNode == _index->_theSkipList->startNode() ||
       nullptr == rNode ) {
    // The index is not empty, the nodes are not neighbours, one of them
    // is at the boundary, so the interval is valid and not empty.
    return true;
  }

  CmpElmElm comparer(_collection->shaper(), _index->nrIndexedFields());
  compareResult = comparer(lNode->document(), rNode->document(), 
                           SkipList_t::CMP_TOTORDER );
  return (compareResult == -1);
  // Since we know that the nodes are not neighbours, we can guarantee
  // at least one document in the interval.
}

////////////////////////////////////////////////////////////////////////////////
/// @brief intervalIntersectionValid
////////////////////////////////////////////////////////////////////////////////

bool SkiplistIndex2::Iterator::intervalIntersectionValid (
                    Interval const& lInterval,
                    Interval const& rInterval,
                    Interval &interval) {
  SkipList_t::Node* lNode;
  SkipList_t::Node* rNode;

  lNode = lInterval._leftEndPoint;
  rNode = rInterval._leftEndPoint;

  if (nullptr == lNode || nullptr == rNode) {
    // At least one left boundary is the end, intersection is empty.
    return false;
  }

  CmpElmElm comparer(_collection->shaper(), _index->nrIndexedFields());
  int compareResult;
  // Now find the larger of the two start nodes:
  if (lNode == _index->_theSkipList->startNode()) {
    // We take rNode, even if it is the start node as well.
    compareResult = -1;
  }
  else if (rNode == _index->_theSkipList->startNode()) {
    // We take lNode
    compareResult = 1;
  }
  else {
    compareResult = comparer(lNode->document(), 
                             rNode->document(), 
                             SkipList_t::CMP_TOTORDER);
  }

  if (compareResult < 1) {
    interval._leftEndPoint = rNode;
  }
  else {
    interval._leftEndPoint = lNode;
  }

  lNode = lInterval._rightEndPoint;
  rNode = rInterval._rightEndPoint;

  // Now find the smaller of the two end nodes:
  if (nullptr == lNode) {
    // We take rNode, even is this also the end node.
    compareResult = 1;
  }
  else if (nullptr == rNode) {
    // We take lNode.
    compareResult = -1;
  }
  else {
    compareResult = comparer(lNode->document(), 
                             rNode->document(),
                             SkipList_t::CMP_TOTORDER);
  }

  if (compareResult < 1) {
    interval._rightEndPoint = lNode;
  }
  else {
    interval._rightEndPoint = rNode;
  }

  return intervalValid(interval);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief getInterval
////////////////////////////////////////////////////////////////////////////////

SkiplistIndex2::Interval SkiplistIndex2::Iterator::getInterval () const {
  return _intervals[_currentInterval];
}

////////////////////////////////////////////////////////////////////////////////
/// @brief hasNext - iterator functionality
////////////////////////////////////////////////////////////////////////////////

bool SkiplistIndex2::Iterator::hasNext () const {
  if (_intervals.empty()) {
    return false;
  }
  if (_reverse) {  // backward iteration
    // Note that _cursor == nullptr if we are before the largest
    // document (i.e. the first one in the iterator)!

    // .........................................................................
    // if we have more intervals than the one we are currently working
    // on then of course we have a previous doc, because intervals are nonempty.
    // .........................................................................
    if (_currentInterval > 0) {
      return true;
    }

    SkipList_t::Node const* leftNode = _index->_theSkipList->prevNode(_cursor);

    // Note that leftNode can be nullptr here!
    // .........................................................................
    // If the leftNode == left end point AND there are no more intervals
    // then we have no next.
    // .........................................................................
    if (leftNode == getInterval()._leftEndPoint) {
      return false;
    }

    return true;
  }
  else {  // forward iteration
    if (_cursor == nullptr) {
      return false;  // indicates end of iterator
    }

    // .........................................................................
    // if we have more intervals than the one we are currently working
    // on then of course we have a next doc, since intervals are nonempty.
    // .........................................................................
    if (_intervals.size() - 1 > _currentInterval) {
      return true;
    }

    SkipList_t::Node const* leftNode = _cursor->nextNode();

    // Note that leftNode can be nullptr here!
    // .........................................................................
    // If the left == right end point AND there are no more intervals
    // then we have no next.
    // .........................................................................
    if (leftNode == getInterval()._rightEndPoint) {
      return false;
    }

    return true;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief next - iterator functionality
////////////////////////////////////////////////////////////////////////////////

SkiplistIndex2::Element* SkiplistIndex2::Iterator::next () {
  // We assume that hasNext() has returned true
  TRI_ASSERT(_currentInterval < _intervals.size());
  Interval interval = getInterval();
  SkipList_t::Node* result = nullptr; 
  if (_reverse) {  // backward iteration

    // .........................................................................
    // use the current cursor and move jumpSize backward
    // .........................................................................

    while (true) {   // will be left by break or return
      result = _index->_theSkipList->prevNode(_cursor);

      if (result == interval._leftEndPoint) {
        if (_currentInterval == 0) {
          _cursor = nullptr;  // exhausted
          return nullptr;
        }
        --_currentInterval;
        interval = getInterval();
        _cursor = interval._rightEndPoint;
        result = _index->_theSkipList->prevNode(_cursor);
      }
      _cursor = result;

      triagens::mvcc::TransactionId from 
          = result->document()->_document->from();
      triagens::mvcc::TransactionId to
          = result->document()->_document->to();
      if (_transaction->isVisibleForRead(from, to)) {
        break;   // we found a prev one
      }
      // Otherwise move on
    }

    TRI_ASSERT(result != nullptr);
    return result->document();
  }
  else {   // forward iteration
    TRI_ASSERT(_cursor != nullptr);  // otherwise hasNext would have said false
    
    // .........................................................................
    // use the current cursor and move jumpSize forward
    // .........................................................................

    while (true) {   // will be left by break
      _cursor = _cursor->nextNode();
      result = _cursor;
      if (result == interval._rightEndPoint) {
        // Note that _cursor can be nullptr here!
        if (_currentInterval == _intervals.size() - 1) {
          _cursor = nullptr;  // exhausted
          return nullptr;
        }
        ++_currentInterval;
        interval = getInterval();
        _cursor = interval._leftEndPoint;
        result = _cursor;
      }
      triagens::mvcc::TransactionId from 
          = result->document()->_document->from();
      triagens::mvcc::TransactionId to
          = result->document()->_document->to();
      if (_transaction->isVisibleForRead(from, to)) {
        break;   // we found the next one
      }
      // Otherwise move on
    }

    TRI_ASSERT(result != nullptr);
    return result->document();
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief skip - iterator functionality
////////////////////////////////////////////////////////////////////////////////

void SkiplistIndex2::Iterator::skip (size_t steps) {
  for (size_t i = 0; i < steps; i++) {
    if (! hasNext()) {
      return;
    }
    Element* elm = next();
    if (elm == nullptr) {
      return;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief insert document into index (called when opening a collection)
////////////////////////////////////////////////////////////////////////////////
        
void SkiplistIndex2::insert (TRI_doc_mptr_t* doc) {
  bool includeForSparse = true;
  Element* listElement = allocateAndFillElement(doc, includeForSparse);

  TRI_ASSERT(listElement != nullptr);

  if (! _sparse || includeForSparse) {
    _theSkipList->insert(listElement);
  }
  else {
    deleteElement(listElement);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief insert document into index
////////////////////////////////////////////////////////////////////////////////
        
void SkiplistIndex2::insert (TransactionCollection* coll,
                             Transaction* trans,
                             TRI_doc_mptr_t* doc) {
  bool includeForSparse = true;
  Element* listElement = allocateAndFillElement(doc, includeForSparse);
    
  if (_sparse && ! includeForSparse) {
    deleteElement(listElement);
    return;
  }
    
  TRI_ASSERT(listElement != nullptr);

  if (! _unique) {
    try {
      WRITE_LOCKER(_lock);
      _theSkipList->insert(listElement);
      return;
    }
    catch (...) {
      deleteElement(listElement);
      throw;
    }
  }
  
  // _unique == true
  // See whether or not we find any revision that is in conflict with us.
  // Note that this could be a revision which we are not allowed to see!
  try {
    WRITE_LOCKER(_lock);

    // Find the last element in the list whose key is less than ours:
    SkipList_t::Node* node = _theSkipList->leftLookup(listElement);
    CmpElmElm comparer(coll->shaper(), nrIndexedFields());
    while (true) {   // will be left by break
      node = node->nextNode();
      if (node == nullptr) {
        break;
      }

      Element* p = node->document();
      if (comparer(listElement, p, SkipList_t::CMP_PREORDER) != 0) {
        break;
      }
      
      TransactionId from = p->_document->from();
      TransactionId to = p->_document->to();
      Transaction::VisibilityType visFrom = trans->visibility(from);
      Transaction::VisibilityType visTo = trans->visibility(to);
      
      if (visFrom == Transaction::VisibilityType::VISIBLE) {
        if (visTo == Transaction::VisibilityType::VISIBLE) {
          continue;  // Ignore, has been made obsolete for us
        }
        else if (visTo == Transaction::VisibilityType::CONCURRENT) {
          THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_MVCC_WRITE_CONFLICT);
        }
        else {  // INVISIBLE, this includes to == 0
          THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_UNIQUE_CONSTRAINT_VIOLATED);
        }
      }
      else if (visFrom == Transaction::VisibilityType::CONCURRENT) {
        if (visTo != Transaction::VisibilityType::VISIBLE ) {
          THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_MVCC_WRITE_CONFLICT);
        }
        else {
          TRI_ASSERT(false);   // to cannot be VISIBLE unless from is
        }
      }
      else {
        TRI_ASSERT(visTo == Transaction::VisibilityType::INVISIBLE);
        // nothing else possible
        continue;
      }
    }

    _theSkipList->insert(listElement);
  }
  catch (...) {
    deleteElement(listElement);
    throw;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief remove document from index
////////////////////////////////////////////////////////////////////////////////
        
TRI_doc_mptr_t* SkiplistIndex2::remove (TransactionCollection*,
                                        Transaction*,
                                        std::string const&,
                                        TRI_doc_mptr_t* doc) {
  return nullptr;   // this is a no-operation
}

////////////////////////////////////////////////////////////////////////////////
/// @brief forget document in the index
////////////////////////////////////////////////////////////////////////////////
        
void SkiplistIndex2::forget (TransactionCollection* coll,
                             Transaction* trans,
                             TRI_doc_mptr_t* doc) {
  bool includeForSparse = false;
  Element* listElement = allocateAndFillElement(doc, includeForSparse);

  if (_sparse && ! includeForSparse) {
    deleteElement(listElement);
    return;
  }

  try {
    WRITE_LOCKER(_lock);
    int res = _theSkipList->remove(listElement);
    
    if (res != TRI_ERROR_NO_ERROR) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_KEYVALUE_KEY_NOT_FOUND);
    }
    deleteElement(listElement);
  }
  catch (...) {
    deleteElement(listElement);
    throw;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief post-insert (does nothing)
////////////////////////////////////////////////////////////////////////////////

void SkiplistIndex2::preCommit (TransactionCollection*, Transaction*) {
  // this is intentionally a no-operation
}

////////////////////////////////////////////////////////////////////////////////
/// @brief determine the amount of memory used by the index
////////////////////////////////////////////////////////////////////////////////

size_t SkiplistIndex2::memory () {
  READ_LOCKER(_lock);
  return _theSkipList->memoryUsage() + elementSize() * _theSkipList->getNrUsed();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return a JSON representation of the index
////////////////////////////////////////////////////////////////////////////////
        
Json SkiplistIndex2::toJson (TRI_memory_zone_t* zone) const {
  Json json = Index::toJson(zone);
  Json fields(zone, Json::Array, _fields.size());
  for (auto& field : _fields) {
    fields.add(Json(zone, field));
  }
  json("fields", fields)
      ("unique", Json(zone, _unique))
      ("sparse", Json(zone, _sparse));
  return json;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief cleanup
////////////////////////////////////////////////////////////////////////////////
        
// a garbage collection function for the index
void SkiplistIndex2::cleanup () {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief sizeHint - ignored
////////////////////////////////////////////////////////////////////////////////
        
// give index a hint about the expected size
void SkiplistIndex2::sizeHint (size_t) {
}

// -----------------------------------------------------------------------------
// --SECTION--                                                 private functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the memory needed for an index key entry
////////////////////////////////////////////////////////////////////////////////

size_t SkiplistIndex2::elementSize () const {
  return sizeof(TRI_doc_mptr_t*) + _paths.size() * sizeof(TRI_shaped_sub_t);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief allocateAndFillElement
////////////////////////////////////////////////////////////////////////////////

SkiplistIndex2::Element* SkiplistIndex2::allocateAndFillElement (
                                 TRI_doc_mptr_t* doc,
                                 bool& includeForSparse) {
  auto elm 
    = static_cast<SkiplistIndex2::Element*>(TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, 
                                                         elementSize(),
                                                         false));

  if (elm == nullptr) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
  }

  elm->_document = doc;

  // ...........................................................................
  // Assign the document to the Element structure - so that it can later
  // be retreived.
  // ...........................................................................

  TRI_shaped_json_t shapedJson;
  TRI_EXTRACT_SHAPED_JSON_MARKER(shapedJson, doc->getDataPtr());

  char const* ptr = doc->getShapedJsonPtr();

  // ...........................................................................
  // Extract the attribute values
  // ...........................................................................

  size_t const n = _paths.size();

  includeForSparse = true;
  for (size_t i = 0; i < n; ++i) {
    TRI_shape_pid_t path = _paths[i];

    // determine if document has that particular shape
    TRI_shape_access_t const* acc 
          = TRI_FindAccessorVocShaper(_collection->getShaper(), shapedJson._sid, path);

    // field not part of the object
    if (acc == nullptr || acc->_resultSid == TRI_SHAPE_ILLEGAL) {
      elm->_subObjects[i]._sid = BasicShapes::TRI_SHAPE_SID_NULL;
      includeForSparse = false;
    }

    // extract the field
    else {
      TRI_shaped_json_t shapedObject;       // the sub-object

      if (! TRI_ExecuteShapeAccessor(acc, &shapedJson, &shapedObject)) {
        TRI_Free(TRI_UNKNOWN_MEM_ZONE, static_cast<void*>(elm));
        THROW_ARANGO_EXCEPTION(TRI_ERROR_INTERNAL);
      }

      if (shapedObject._sid == BasicShapes::TRI_SHAPE_SID_NULL) {
        includeForSparse = false;
      }

      TRI_FillShapedSub(&elm->_subObjects[i], &shapedObject, ptr);
    }
  }

  return elm;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief deleteElement
////////////////////////////////////////////////////////////////////////////////

void SkiplistIndex2::deleteElement (Element* elm) {
  TRI_Free(TRI_UNKNOWN_MEM_ZONE, static_cast<void*>(elm));
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
