////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC index user class
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

#include "IndexUser.h"
#include "Mvcc/Index.h"
#include "Mvcc/PrimaryIndex.h"
#include "Mvcc/TransactionCollection.h"
#include "Utils/Exception.h"
#include "VocBase/document-collection.h"

using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                                   class IndexUser
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief read-locks the lists of indexes of a collection
////////////////////////////////////////////////////////////////////////////////

IndexUser::IndexUser (TransactionCollection* collection)
  : _collection(collection),
    _indexes(),
    _mustClick(false) {

  _collection->documentCollection()->readLockIndexes();
  _indexes = _collection->documentCollection()->indexes();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief give up index usage
////////////////////////////////////////////////////////////////////////////////

IndexUser::~IndexUser () {
  if (_mustClick) {
    click();
  }

  _collection->documentCollection()->readUnlockIndexes();
}

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not the collection has a cap constraint
////////////////////////////////////////////////////////////////////////////////

bool IndexUser::hasCapConstraint () const {
  size_t const n = _indexes.size();
  for (size_t i = 1; i < n; ++i) {
    if (_indexes[i]->type() == triagens::mvcc::Index::TRI_IDX_TYPE_CAP_CONSTRAINT) {
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not the collection has secondary indexes
////////////////////////////////////////////////////////////////////////////////

bool IndexUser::hasSecondaryIndexes () const {
  return _indexes.size() > 1;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return a reference to the primary index
////////////////////////////////////////////////////////////////////////////////

triagens::mvcc::PrimaryIndex* IndexUser::primaryIndex () const {
  if (_indexes.empty()) {
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_INTERNAL, "no primary index found");
  }

  return static_cast<triagens::mvcc::PrimaryIndex*>(_indexes[0]);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief "click" all indexes to enforce a synchronization
////////////////////////////////////////////////////////////////////////////////

void IndexUser::click () {
  for (auto it : _indexes) {
    try {
      it->clickLock();
    }
    catch (...) {
    }
  }

  _mustClick = false;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
