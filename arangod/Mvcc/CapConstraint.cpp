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

#include "CapConstraint.h"
#include "Mvcc/CollectionOperations.h"
#include "Mvcc/MasterpointerManager.h"
#include "Mvcc/TransactionCollection.h"
#include "Utils/Exception.h"
#include "Utils/transactions.h"
#include "VocBase/datafile.h"
#include "VocBase/document-collection.h"

using namespace triagens::basics;
using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                               class CapConstraint
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new cap constraint
////////////////////////////////////////////////////////////////////////////////

CapConstraint::CapConstraint (TRI_idx_iid_t id,
                              TRI_document_collection_t* collection,
                              size_t count,
                              int64_t size)
  : Index(id, collection, std::vector<std::string>()),
    _count(count),
    _size(static_cast<int64_t>(size)) {

}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the cap constraint
////////////////////////////////////////////////////////////////////////////////

CapConstraint::~CapConstraint () {
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions 
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief insert a document when loading a collection
////////////////////////////////////////////////////////////////////////////////

void CapConstraint::insert (TRI_doc_mptr_t* doc) {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief insert a document
////////////////////////////////////////////////////////////////////////////////

void CapConstraint::insert (TransactionCollection* collection,
                            Transaction* transaction,
                            TRI_doc_mptr_t* doc) {
  if (_size > 0) {
    // there is a size restriction
    auto marker = static_cast<TRI_df_marker_t const*>(doc->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

    // check if the document would be too big
    if (static_cast<int64_t>(marker->_size) > _size) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_DOCUMENT_TOO_LARGE);
    }
  }

  int res = apply(collection, transaction, true);

  if (res != TRI_ERROR_NO_ERROR) {
    THROW_ARANGO_EXCEPTION(res);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief remove a document (does nothing)
////////////////////////////////////////////////////////////////////////////////

TRI_doc_mptr_t* CapConstraint::remove (TransactionCollection*, 
                                       Transaction*,
                                       std::string const&,
                                       TRI_doc_mptr_t*) {
  return nullptr; 
}  

////////////////////////////////////////////////////////////////////////////////
/// @brief forget a document (does nothing)
////////////////////////////////////////////////////////////////////////////////

void CapConstraint::forget (TransactionCollection*, 
                            Transaction*,
                            TRI_doc_mptr_t*) {
}  

////////////////////////////////////////////////////////////////////////////////
/// @brief preCommit a document (does nothing so far)
////////////////////////////////////////////////////////////////////////////////

void CapConstraint::preCommit (TransactionCollection*,
                               Transaction*) {
}  

////////////////////////////////////////////////////////////////////////////////
/// @brief return the amount of memory used by the index
////////////////////////////////////////////////////////////////////////////////

size_t CapConstraint::memory () {
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return a JSON representation of the index
////////////////////////////////////////////////////////////////////////////////
        
Json CapConstraint::toJson (TRI_memory_zone_t* zone) const {
  Json json = Index::toJson(zone);
  json("size", Json(zone, static_cast<double>(_count)))
      ("byteSize", Json(zone, static_cast<double>(_size)));
  return json;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief apply the cap constraint for the collection
////////////////////////////////////////////////////////////////////////////////

int CapConstraint::apply (TransactionCollection* collection,
                          Transaction* transaction,
                          bool invokedOnInsert) {
  auto stats = transaction->aggregatedStats(collection->id());

  // get number of documents
  int64_t currentCount = collection->documentCount();
  currentCount += stats.numInserted;
  currentCount -= stats.numRemoved;

  if (invokedOnInsert) {
    ++currentCount;
  }

  // get size of documents
  int64_t currentSize = collection->documentSize();
  currentSize += stats.sizeInserted;
  currentSize -= stats.sizeRemoved;

  auto masterpointerManager = collection->masterpointerManager();
  
  MasterpointerIterator iterator(transaction, masterpointerManager, false);

  // keep removing while at least one of the constraints is violated
  while ((_count > 0 && currentCount > _count) ||
         (_size > 0 && currentSize > _size)) {

    if (! iterator.hasMore()) {
      return TRI_ERROR_NO_ERROR;
    }

    auto mptr = iterator.next();

    if (mptr == nullptr) {
      return TRI_ERROR_NO_ERROR;
    }

    auto result = CollectionOperations::RemoveDocument(transaction, collection, const_cast<TRI_doc_mptr_t*>(mptr)); 


    if (result.code != TRI_ERROR_NO_ERROR) {
      return result.code;
    }

    transaction->incNumRemoved(collection, 1, result.mptr->getDataSize(), false);
    transaction->updateRevisionId(collection, result.tick);

    // update local stats
    currentSize -= result.mptr->getDataSize();
    --currentCount;
  }

  return TRI_ERROR_NO_ERROR;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
