////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC indexes read locker class
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

#include "IndexesReadLocker.h"
#include "Mvcc/Index.h"
#include "Mvcc/TransactionCollection.h"
#include "VocBase/document-collection.h"

using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                           class IndexesReadLocker
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief read-locks the lists of indexes of a collection
////////////////////////////////////////////////////////////////////////////////

IndexesReadLocker::IndexesReadLocker (TransactionCollection* collection)
  : _collection(collection) {

  _collection->documentCollection()->writeLockIndexes();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief give up index usage
////////////////////////////////////////////////////////////////////////////////

IndexesReadLocker::~IndexesReadLocker () {
  _collection->documentCollection()->writeUnlockIndexes();
}

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief return the list of indexes
////////////////////////////////////////////////////////////////////////////////

std::vector<triagens::mvcc::Index*> IndexesReadLocker::indexes () const {
  return _collection->documentCollection()->indexes();
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
