////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC collection operations
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

#include "CollectionOperations.h"
#include "Basics/json.h"
#include "Basics/json-utilities.h"
#include "Cluster/ClusterMethods.h"
#include "Mvcc/CollectionReadLock.h"
#include "Mvcc/Index.h"
#include "Mvcc/MasterpointerManager.h"
#include "Mvcc/PrimaryIndex.h"
#include "Mvcc/TransactionScope.h"
#include "ShapedJson/Legends.h"
#include "ShapedJson/shaped-json.h"
#include "Utils/Exception.h"
#include "VocBase/document-collection.h"
#include "VocBase/server.h"
#include "VocBase/vocbase.h"
#include "VocBase/voc-shaper.h"
#include "Wal/LogfileManager.h"
#include "Wal/Marker.h"

using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                          private static variables
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief default search options used when none are specified
////////////////////////////////////////////////////////////////////////////////

static SearchOptions const DefaultSearchOptions;

////////////////////////////////////////////////////////////////////////////////
/// @brief a transaction id used for single-operation transactions
////////////////////////////////////////////////////////////////////////////////

static TransactionId const DefaultTransactionId = { 0, 0 };

// -----------------------------------------------------------------------------
// --SECTION--                                                   struct Document
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create an empty document container
////////////////////////////////////////////////////////////////////////////////

Document::Document (TRI_shaper_t* shaper,
                    TRI_shaped_json_t const* shaped,
                    std::string const& key,
                    TRI_voc_rid_t revision,
                    bool keySpecified,
                    bool freeShape) 
  : shaper(shaper),
    shaped(shaped),
    key(key),
    revision(revision),
    edge(nullptr),
    keySpecified(keySpecified),
    freeShape(freeShape) {

  TRI_ASSERT(shaped == nullptr || shaper != nullptr);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a document from a master pointer
////////////////////////////////////////////////////////////////////////////////

Document::Document (TRI_doc_mptr_t const* mptr)
  : shaper(nullptr),
    shaped(nullptr),
    key(TRI_EXTRACT_MARKER_KEY(mptr)),
    revision(TRI_EXTRACT_MARKER_RID(mptr)),
    edge(nullptr),
    keySpecified(false),
    freeShape(false) {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a document from another
////////////////////////////////////////////////////////////////////////////////
        
Document::Document (Document&& other) 
  : shaper(other.shaper),
    shaped(other.shaped),
    key(other.key),
    revision(other.revision),
    edge(nullptr),
    keySpecified(other.keySpecified),
    freeShape(other.freeShape) {

  // do not double-free
  other.shaped    = nullptr;
  other.freeShape = false;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy a document container
////////////////////////////////////////////////////////////////////////////////

Document::~Document () {
  if (shaped != nullptr && freeShape) {
    TRI_ASSERT(shaper != nullptr);
    TRI_FreeShapedJson(shaper->_memoryZone, const_cast<TRI_shaped_json_t*>(shaped));
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief get the internals as JSON
////////////////////////////////////////////////////////////////////////////////

TRI_json_t* Document::toJson () const {
  if (shaper == nullptr || shaped == nullptr) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_INTERNAL);
  }
      
  return TRI_JsonShapedJson(shaper, shaped);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a document from JSON
////////////////////////////////////////////////////////////////////////////////

Document Document::CreateFromJson (TRI_shaper_t* shaper,
                                   TRI_json_t const* json) {
         
  if (! TRI_IsObjectJson(json)) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_DOCUMENT_TYPE_INVALID);
  }
 
  std::string keyString;
   
  // check if _key is there
  TRI_json_t const* key = TRI_LookupObjectJson(json, TRI_VOC_ATTRIBUTE_KEY);
  
  if (key != nullptr) {
    if (! TRI_IsStringJson(key)) {
      // _key is there but not a string
      THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_DOCUMENT_KEY_BAD);
    }
    
    keyString.assign(key->_value._string.data, key->_value._string.length - 1);
  }
  
  auto* shaped = TRI_ShapedJsonJson(shaper, json, true);
  
  if (shaped == nullptr) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_SHAPER_FAILED);
  }

  try {
    return Document(shaper, 
                    shaped, 
                    keyString,
                    0, // TODO: revision 
                    key == nullptr ? CollectionOperations::NoKeySpecified : CollectionOperations::KeySpecified, 
                    true);
  }
  catch (...) {
    TRI_FreeShapedJson(shaper->_memoryZone, shaped);
    throw;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a document from JSON, with key specified separately
////////////////////////////////////////////////////////////////////////////////

Document Document::CreateFromJson (TRI_shaper_t* shaper,
                                   TRI_json_t const* json,
                                   std::string const& key,
                                   TRI_voc_rid_t revisionId) {
         
  if (! TRI_IsObjectJson(json)) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_DOCUMENT_TYPE_INVALID);
  }
 
  auto* shaped = TRI_ShapedJsonJson(shaper, json, true);
  
  if (shaped == nullptr) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_SHAPER_FAILED);
  }

  try {
    return Document(shaper, 
                    shaped, 
                    key,
                    revisionId,
                    CollectionOperations::KeySpecified, 
                    true);
  }
  catch (...) {
    TRI_FreeShapedJson(shaper->_memoryZone, shaped);
    throw;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a document from ShapedJson
////////////////////////////////////////////////////////////////////////////////

Document Document::CreateFromShapedJson (TRI_shaper_t* shaper,
                                         TRI_shaped_json_t const* shaped,
                                         char const* key,
                                         bool freeShape) {
  if (shaped == nullptr) {
    // the shaped data is invalid
    THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_DOCUMENT_TYPE_INVALID);
  }

  try {
    std::string keyString;

    if (key != nullptr) {         
      // user has specified a key
      keyString.assign(key);
    }
     
    return Document(shaper, 
                    shaped, 
                    keyString,
                    0, // TODO: revision 
                    key == nullptr ? CollectionOperations::NoKeySpecified : CollectionOperations::KeySpecified, 
                    freeShape);
  }
  catch (...) {
    // make sure that the shape gets deleted in every case
    if (freeShape) {
      TRI_FreeShapedJson(shaper->_memoryZone, const_cast<TRI_shaped_json_t*>(shaped));
    }

    throw;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a document from a key only
////////////////////////////////////////////////////////////////////////////////

Document Document::CreateFromKey (std::string const& key,
                                  TRI_voc_rid_t revisionId) {

  return Document(nullptr, nullptr, key, revisionId, CollectionOperations::KeySpecified, false);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a document from a master pointer
////////////////////////////////////////////////////////////////////////////////

Document Document::CreateFromMptr (TRI_doc_mptr_t const* mptr) {
  return Document(mptr);
}

// -----------------------------------------------------------------------------
// --SECTION--                                       struct CollectionOperations
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief return the number of documents in the collection
////////////////////////////////////////////////////////////////////////////////

int64_t CollectionOperations::Count (TransactionScope* transactionScope,
                                     TransactionCollection* collection) {
  auto* transaction = transactionScope->transaction();

  // initial value
  int64_t count = collection->documentCount();
  auto stats = transaction->aggregatedStats(collection->id());
  count += stats.numInserted;
  count -= stats.numRemoved;

  return count;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the size of documents in the collection
////////////////////////////////////////////////////////////////////////////////

int64_t CollectionOperations::Size (TransactionScope* transactionScope,
                                    TransactionCollection* collection) {
  auto* transaction = transactionScope->transaction();

  // initial value
  int64_t size = collection->documentSize();
  auto stats = transaction->aggregatedStats(collection->id());
  size += stats.sizeInserted;
  size -= stats.sizeRemoved;

  return size;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the latest revision id the collection
////////////////////////////////////////////////////////////////////////////////

TRI_voc_rid_t CollectionOperations::Revision (TransactionScope* transactionScope,
                                              TransactionCollection* collection) {
  auto* transaction = transactionScope->transaction();

  // initial value
  TRI_voc_rid_t revisionId = collection->revisionId();
  auto stats = transaction->aggregatedStats(collection->id());

  if (stats.revisionId > revisionId) {
    return stats.revisionId;
  }

  return revisionId;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief reads a random document from the collection
////////////////////////////////////////////////////////////////////////////////

OperationResult CollectionOperations::RandomDocument (TransactionScope* transactionScope,
                                                      TransactionCollection* collection) {
  // acquire a read-lock on the list of indexes so no one else creates or drops indexes
  // while the operation is ongoing
  IndexUser indexUser(collection);
  
  auto primaryIndex = indexUser.primaryIndex();
 
  auto* transaction = transactionScope->transaction();
  auto mptr = primaryIndex->random(collection, transaction);
  
  // no need to commit a read, but if we don't commit, it would be rolled back 
  // and rollbacks make you inspect logs too often unnecessarily
  transactionScope->commit();

  if (mptr == nullptr) {
    return OperationResult(TRI_ERROR_NO_ERROR);
  }

  TRI_ASSERT(mptr != nullptr);
    
  return OperationResult(mptr);  
}

////////////////////////////////////////////////////////////////////////////////
/// @brief truncate the collection
////////////////////////////////////////////////////////////////////////////////

OperationResult CollectionOperations::Truncate (TransactionScope* transactionScope,
                                                TransactionCollection* collection,
                                                OperationOptions const& options) {
  auto* transaction = transactionScope->transaction();
  transaction->prepareStats(collection);

  TRI_ASSERT(! options.standAlone);

  // make sure we are holding a read (sic!) lock so we do not interfere with other
  // operations that explicitly lock the collection
  CollectionReadLock lock(collection, transaction);
  
  // acquire a read-lock on the list of indexes so no one else creates or drops indexes
  // while the operation is ongoing
  IndexUser indexUser(collection);

  int64_t numRemoved  = 0;
  int64_t sizeRemoved = 0;
  TRI_voc_rid_t tick  = 0;

  {
    std::unique_ptr<MasterpointerIterator> iterator(new MasterpointerIterator(transaction, collection->masterpointerManager(), false));
    auto it = iterator.get();
  
    TransactionId originalTransactionId;
   
    while (true) {
      TRI_doc_mptr_t const* found = it->next();

      if (found == nullptr) {
        break;
      }
  
      auto result = RemoveDocumentWorker(transactionScope, collection, indexUser, Document::CreateFromMptr(found), options, originalTransactionId);
  
      if (result.code != TRI_ERROR_NO_ERROR) {
        return result;
      }
        
      numRemoved++;
      sizeRemoved += result.mptr->getDataSize();
      tick = result.tick;
    }
  }

  if (numRemoved > 0) {
    TRI_ASSERT(sizeRemoved > 0);
    transaction->incNumRemoved(collection, numRemoved, sizeRemoved, options.waitForSync);
    transaction->updateRevisionId(collection, tick);
  }

  return OperationResult(TRI_ERROR_NO_ERROR, transaction->hasWaitForSync());
}

////////////////////////////////////////////////////////////////////////////////
/// @brief insert a document
/// this calls the internal worker function for insert
////////////////////////////////////////////////////////////////////////////////

OperationResult CollectionOperations::InsertDocument (TransactionScope* transactionScope,
                                                      TransactionCollection* collection,
                                                      Document& document,
                                                      OperationOptions const& options) {
  auto* transaction = transactionScope->transaction();
  transaction->prepareStats(collection);
  
  // make sure we are holding a read (sic!) lock so we do not interfere with other
  // operations that explicitly lock the collection
  CollectionReadLock lock(collection, transaction);

  // acquire a read-lock on the list of indexes so no one else creates or drops indexes
  // while the operation is ongoing
  IndexUser indexUser(collection);
 
  // if the transaction will only contain this single operation, we can perform an
  // optimization that will spare the commit marker 
  if (options.standAlone &&
      transactionScope->hasStartedTransaction() &&
      ! indexUser.hasCapConstraint()) {
    transaction->singleOperation(true);
  }
  
  auto result = InsertDocumentWorker(transactionScope, collection, indexUser, document, options);

  if (result.code == TRI_ERROR_NO_ERROR) {
    // commit the local operation
    transaction->incNumInserted(collection, 1, result.mptr->getDataSize(), options.waitForSync);
    transaction->updateRevisionId(collection, result.tick);
    transactionScope->commit();
  }
 
  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief reads a document by its key
/// this calls the internal worker function for read
////////////////////////////////////////////////////////////////////////////////

OperationResult CollectionOperations::ReadDocument (TransactionScope* transactionScope,
                                                    TransactionCollection* collection,
                                                    Document const& document,
                                                    OperationOptions const& options) {
  // acquire a read-lock on the list of indexes so no one else creates or drops indexes
  // while the operation is ongoing
  IndexUser indexUser(collection);

  auto result = ReadDocumentWorker(transactionScope, collection, indexUser, document, options);

  if (result.code == TRI_ERROR_NO_ERROR) {
    // no need to commit a read, but if we don't commit, it would be rolled back 
    // and rollbacks make you inspect logs too often unnecessarily
    transactionScope->commit();
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief read all documents of the collection
////////////////////////////////////////////////////////////////////////////////

OperationResult CollectionOperations::ReadAllDocuments (TransactionScope* transactionScope,
                                                        TransactionCollection* collection,
                                                        std::vector<TRI_doc_mptr_t const*>& foundDocuments,
                                                        OperationOptions const& options) {
  auto* transaction = transactionScope->transaction();

  // must have search options!
  SearchOptions const* searchOptions = options.searchOptions;
  if (options.searchOptions == nullptr) {
    searchOptions = &DefaultSearchOptions;
  }

  auto skip = searchOptions->skip;
  auto limit = searchOptions->limit;
  bool reverse = searchOptions->reverse;

  {
    std::unique_ptr<MasterpointerIterator> iterator(new MasterpointerIterator(transaction, collection->masterpointerManager(), reverse));
    auto it = iterator.get();
   
    while (it->next(foundDocuments, skip, limit, 1000))
      ;
  }

  OperationResult result(TRI_ERROR_NO_ERROR);
 
  if (result.code == TRI_ERROR_NO_ERROR) {
    // no need to commit a read, but if we don't commit, it would be rolled back 
    // and rollbacks make you inspect logs too often unnecessarily
    transactionScope->commit();
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief read all documents by example
////////////////////////////////////////////////////////////////////////////////

OperationResult CollectionOperations::ReadByExample (TransactionScope* transactionScope,
                                                     TransactionCollection* collection,
                                                     std::vector<TRI_shape_pid_t> const& pids,
                                                     std::vector<TRI_shaped_json_t*> const& shapes,
                                                     std::vector<TRI_doc_mptr_t const*>& foundDocuments,
                                                     OperationOptions const& options) {
  auto* transaction = transactionScope->transaction();

  // must have search options!
  TRI_ASSERT(options.searchOptions != nullptr);

  auto skip = options.searchOptions->skip;
  auto limit = options.searchOptions->limit;
  bool reverse = options.searchOptions->reverse;

  auto shaper = collection->shaper();

  TRI_ASSERT(shapes.size() == pids.size());

  auto isMatch = [&shaper, &shapes, &pids](TRI_shaped_json_t const* shaped) -> bool {
    TRI_shape_t const* shape;
    TRI_shaped_json_t result;
  
    size_t const n = shapes.size();

    for (size_t i = 0; i < n; ++i) {
      bool ok = TRI_ExtractShapedJsonVocShaper(shaper,
                                               shaped,
                                               shapes[i]->_sid,
                                               pids[i],
                                               &result,
                                               &shape);
    
      if (! ok || shape == nullptr) {
        return false;
      }

      auto example = shapes[i]->_data;
    
      if (result._data.length != example.length) {
        return false;
      }

      if (::memcmp(result._data.data, example.data, example.length) != 0) {
        return false;
      }
    }

    return true;
  };


  {
    TRI_shaped_json_t shaped;

    std::unique_ptr<MasterpointerIterator> iterator(new MasterpointerIterator(transaction, collection->masterpointerManager(), reverse));
    auto it = iterator.get();
   
    while (true) {
      auto document = it->next(skip, limit);

      if (document == nullptr) {
        break;
      }
  
      TRI_EXTRACT_SHAPED_JSON_MARKER(shaped, document->getDataPtr());  // PROTECTED by trx coming from above

      if (isMatch(&shaped)) {
        foundDocuments.push_back(document);
      }
    }
  }

  OperationResult result(TRI_ERROR_NO_ERROR);
 
  if (result.code == TRI_ERROR_NO_ERROR) {
    // no need to commit a read, but if we don't commit, it would be rolled back 
    // and rollbacks make you inspect logs too often unnecessarily
    transactionScope->commit();
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief remove a document
/// this calls the internal worker function for remove
////////////////////////////////////////////////////////////////////////////////

OperationResult CollectionOperations::RemoveDocument (TransactionScope* transactionScope,
                                                      TransactionCollection* collection,
                                                      Document const& document,
                                                      OperationOptions const& options) {
  auto* transaction = transactionScope->transaction();
  transaction->prepareStats(collection);
  
  // make sure we are holding a read (sic!) lock so we do not interfere with other
  // operations that explicitly lock the collection
  CollectionReadLock lock(collection, transaction);

  // acquire a read-lock on the list of indexes so no one else creates or drops indexes
  // while the operation is ongoing
  IndexUser indexUser(collection);
  
  // if the transaction will only contain this single operation, we can perform an
  // optimization that will spare the commit marker 
  if (options.standAlone &&
      transactionScope->hasStartedTransaction() &&
      ! indexUser.hasCapConstraint()) {
    transaction->singleOperation(true);
  }

  TransactionId originalTransactionId;
  auto result = RemoveDocumentWorker(transactionScope, collection, indexUser, document, options, originalTransactionId);

  if (result.code == TRI_ERROR_NO_ERROR) {
    // commit the local operation
    transaction->incNumRemoved(collection, 1, result.mptr->getDataSize(), options.waitForSync);
    transaction->updateRevisionId(collection, result.tick);
    transactionScope->commit();
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief remove a document
////////////////////////////////////////////////////////////////////////////////

OperationResult CollectionOperations::RemoveDocument (Transaction* transaction,
                                                      TransactionCollection* collection,
                                                      TRI_doc_mptr_t* document) {
  auto const transactionId = transaction->id();
  
  // make sure we are holding a read (sic!) lock so we do not interfere with other
  // operations that explicitly lock the collection
  CollectionReadLock lock(collection, transaction);

  // remove document from primary index
  // this fetches the master pointer of the to-be-deleted revision
  // and sets its _to value to our transaction id
  IndexUser indexUser(collection);

  auto primaryIndex = indexUser.primaryIndex();
  TransactionId originalTransactionId;
  
  auto key = TRI_EXTRACT_MARKER_KEY(document);
  TRI_ASSERT(key != nullptr);

  // this throws on error
  auto mptr = primaryIndex->remove(collection, transaction, key, originalTransactionId);
  TRI_ASSERT(mptr != nullptr);
  indexUser.mustClick();

  TRI_voc_tick_t tick;
  try {
    // iterate over all secondary indexes while holding the index read-lock
    auto indexes = indexUser.indexes();

    for (size_t i = 1; i < indexes.size(); ++i) {
      // call remove for each secondary indexes
      indexes[i]->remove(collection, transaction, key, mptr);
    }
    
    // when we are here, the document has been removed from all indexes 

    // create a temporary remove marker on the heap
    // the marker memory will be freed automatically when we leave this method
    std::unique_ptr<triagens::wal::MvccRemoveMarker> marker(
      new triagens::wal::MvccRemoveMarker(transaction->vocbase()->_id,
                                          collection->id(),
                                          mptr->_rid,
                                          transaction->singleOperation() ? DefaultTransactionId : transactionId,
                                          key)
    );

    // now append the marker to the WAL
    WriteResult writeResult;
    int res = WriteMarker(marker.get(), collection, writeResult);

    if (res != TRI_ERROR_NO_ERROR) {
      THROW_ARANGO_EXCEPTION(res);
    }

    tick = writeResult.tick;
  }
  catch (...) {
    // revert the value of the _to attribute
    if (! mptr->changeTo(transactionId, originalTransactionId)) {
      // oops, should not happen
      TRI_ASSERT(false);
    }

    // TODO: do we need to undo the remove() in the secondary indexes, too??
    throw;
  }

  TRI_ASSERT(mptr != nullptr);

  return OperationResult(mptr, tick, transaction->hasWaitForSync());
}

////////////////////////////////////////////////////////////////////////////////
/// @brief update a document
////////////////////////////////////////////////////////////////////////////////

OperationResult CollectionOperations::UpdateDocument (TransactionScope* transactionScope,
                                                      TransactionCollection* collection,
                                                      Document& document,
                                                      TRI_json_t const* update,
                                                      OperationOptions const& options) {
  auto* transaction = transactionScope->transaction();
  transaction->prepareStats(collection);
  
  // make sure we are holding a read (sic!) lock so we do not interfere with other
  // operations that explicitly lock the collection
  CollectionReadLock lock(collection, transaction);

  // acquire a read-lock on the list of indexes so no one else creates or drops indexes
  // while the operation is ongoing
  IndexUser indexUser(collection);
  
  // if the transaction will only contain this single operation, we can perform an
  // optimization that will spare the commit marker 
  if (options.standAlone &&
      transactionScope->hasStartedTransaction() &&
      ! indexUser.hasCapConstraint()) {
    transaction->singleOperation(true);
  }

  // first remove the document...
  TransactionId originalTransactionId;
  auto removeResult = RemoveDocumentWorker(transactionScope, collection, indexUser, document, options, originalTransactionId);

  if (removeResult.code != TRI_ERROR_NO_ERROR) {
    return removeResult;
  }

  // we have found and removed a revision!
  TRI_ASSERT(removeResult.mptr != nullptr);

  try {
    // convert the existing (shaped) document to JSON
    TRI_shaped_json_t shaped;
    TRI_EXTRACT_SHAPED_JSON_MARKER(shaped, removeResult.mptr->getDataPtr());  // PROTECTED by trx here
    std::unique_ptr<TRI_json_t> previous(TRI_JsonShapedJson(collection->shaper(), &shaped));  // PROTECTED by trx here

    if (previous.get() == nullptr) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
    }
  
    if (triagens::arango::ServerState::instance()->isDBserver()) {
      // compare attributes in shardKeys
      if (triagens::arango::shardKeysChanged(collection->dbName(), std::to_string(collection->planId()), previous.get(), update, true)) {
        THROW_ARANGO_EXCEPTION(TRI_ERROR_CLUSTER_MUST_NOT_CHANGE_SHARDING_ATTRIBUTES);
      }
    }

    // merge existing document with new data
    std::unique_ptr<TRI_json_t> merged(TRI_MergeJson(TRI_UNKNOWN_MEM_ZONE, previous.get(), update, ! options.keepNull, options.mergeObjects));

    if (merged.get() == nullptr) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
    }
  
    // now insert the updated document
    auto updateDocument = Document::CreateFromJson(collection->shaper(), merged.get(), document.key); 
    auto result = InsertDocumentWorker(transactionScope, collection, indexUser, updateDocument, options);

    if (result.code != TRI_ERROR_NO_ERROR) {
      return result;
    }

    // in this case, actualRevision contains the revision id of the updated document 
    result.actualRevision = removeResult.mptr->_rid;
  
    try {
      // commit the local operation
      transaction->incNumRemoved(collection, 1, removeResult.mptr->getDataSize(), options.waitForSync);
      transaction->incNumInserted(collection, 1, result.mptr->getDataSize(), options.waitForSync);
      transaction->updateRevisionId(collection, result.tick);
      transactionScope->commit();
    }
    catch (...) {
      // revert the insert operation!
      auto indexes = indexUser.indexes();
      for (size_t i = 0; i < indexes.size(); ++i) {
        indexes[i]->forget(collection, transaction, result.mptr);
      }
      
      throw;
    }

    return result;
  }
  catch (...) {
    // must revert the remove operation!
    const_cast<TRI_doc_mptr_t*>(removeResult.mptr)->changeTo(transaction->id(), originalTransactionId);
    throw;
  }
   
  // unreachable
  TRI_ASSERT(false); 
}

////////////////////////////////////////////////////////////////////////////////
/// @brief replace a document
////////////////////////////////////////////////////////////////////////////////

OperationResult CollectionOperations::ReplaceDocument (TransactionScope* transactionScope,
                                                       TransactionCollection* collection,
                                                       Document& document,
                                                       OperationOptions const& options) {
  auto* transaction = transactionScope->transaction();
  transaction->prepareStats(collection);
  
  // make sure we are holding a read (sic!) lock so we do not interfere with other
  // operations that explicitly lock the collection
  CollectionReadLock lock(collection, transaction);

  // acquire a read-lock on the list of indexes so no one else creates or drops indexes
  // while the operation is ongoing
  IndexUser indexUser(collection);
  
  // if the transaction will only contain this single operation, we can perform an
  // optimization that will spare the commit marker 
  if (options.standAlone &&
      transactionScope->hasStartedTransaction() &&
      ! indexUser.hasCapConstraint()) {
    transaction->singleOperation(true);
  }

  // first remove the document...
  TransactionId originalTransactionId;
  auto removeResult = RemoveDocumentWorker(transactionScope, collection, indexUser, document, options, originalTransactionId);

  if (removeResult.code != TRI_ERROR_NO_ERROR) {
    return removeResult;
  }

  // we have found and removed a revision!
  TRI_ASSERT(removeResult.mptr != nullptr);

  try {
    if (triagens::arango::ServerState::instance()->isDBserver()) {
      // compare attributes in shardKeys

      // extract existing document
      TRI_shaped_json_t shaped;
      TRI_EXTRACT_SHAPED_JSON_MARKER(shaped, removeResult.mptr->getDataPtr());
      std::unique_ptr<TRI_json_t> previous(TRI_JsonShapedJson(collection->shaper(), &shaped));
      
      std::unique_ptr<TRI_json_t> replace(document.toJson());

      if (previous.get() == nullptr || replace.get() == nullptr) {
        THROW_ARANGO_EXCEPTION(TRI_ERROR_INTERNAL);
      }

      if (triagens::arango::shardKeysChanged(collection->dbName(), std::to_string(collection->planId()), previous.get(), replace.get(), false)) {
        THROW_ARANGO_EXCEPTION(TRI_ERROR_CLUSTER_MUST_NOT_CHANGE_SHARDING_ATTRIBUTES);
      }
    }

    // now insert the replacement document
    auto result = InsertDocumentWorker(transactionScope, collection, indexUser, document, options);

    if (result.code != TRI_ERROR_NO_ERROR) {
      return result;
    }

    // in this case, actualRevision contains the revision id of the updated document 
    result.actualRevision = removeResult.mptr->_rid;
  
    try {
      // commit the local operation
      transaction->incNumRemoved(collection, 1, removeResult.mptr->getDataSize(), options.waitForSync);
      transaction->incNumInserted(collection, 1, result.mptr->getDataSize(), options.waitForSync);
      transaction->updateRevisionId(collection, result.tick);
      transactionScope->commit();
    }
    catch (...) {
      // revert the insert operation!
      auto indexes = indexUser.indexes();
      for (size_t i = 0; i < indexes.size(); ++i) {
        indexes[i]->forget(collection, transaction, result.mptr);
      }
      
      throw;
    }

    return result;
  }
  catch (...) {
    // must revert the remove operation!
    const_cast<TRI_doc_mptr_t*>(removeResult.mptr)->changeTo(transaction->id(), originalTransactionId);
    throw;
  }
   
  // unreachable
  TRI_ASSERT(false); 
}

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief insert a document
////////////////////////////////////////////////////////////////////////////////

OperationResult CollectionOperations::InsertDocumentWorker (TransactionScope* transactionScope,
                                                            TransactionCollection* collection,
                                                            IndexUser& indexUser,
                                                            Document& document,
                                                            OperationOptions const& options) {

  auto* transaction = transactionScope->transaction();
  auto const transactionId = transaction->id();

  // create a revision for the document
  if (document.revision == 0) {
    document.revision = TRI_NewTickServer();
  }
  
  if (document.shaped == nullptr) {
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_INTERNAL, "should not insert a document without body");
  }
  
  // valdidate the document _key or create a new one
  CreateOrValidateKey(collection, &document);
  
  // create a temporary marker for the document on the heap
  // the marker memory will be freed automatically when we leave this method
  std::unique_ptr<triagens::wal::Marker> marker;
  
  if (document.edge != nullptr) {
    marker.reset(new triagens::wal::MvccEdgeMarker(transaction->vocbase()->_id,
                                                   collection->id(),
                                                   document.revision,
                                                   transaction->singleOperation() ? DefaultTransactionId : transactionId,
                                                   document.key,
                                                   document.edge,
                                                   8, /* legendSize */
                                                   document.shaped)
    );
  }
  else {
    marker.reset(new triagens::wal::MvccDocumentMarker(transaction->vocbase()->_id,
                                                       collection->id(),
                                                       document.revision,
                                                       transaction->singleOperation() ? DefaultTransactionId : transactionId,
                                                       document.key,
                                                       8, /* legendSize */
                                                       document.shaped)
    );
  }
  
  // create a master pointer which will hold the marker
  // this will automatically release the master pointer when we leave this method 
  // and have not linked the master pointer
  
  // set data pointer to point to the marker just created
  MasterpointerContainer mptr = collection->masterpointerManager()->create(marker.get()->mem(), transactionId);
  mptr->setFrom(transactionId);

  TRI_IF_FAILURE("CollectionOperations::InsertDocumentWorker1") {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_DEBUG);
  }
  
  // iterate over all indexes while holding the index read-lock
  auto indexes = indexUser.indexes();
  for (size_t i = 0; i < indexes.size(); ++i) {
    // and call insert for each index
    try {
      indexes[i]->insert(collection, transaction, *mptr);
      indexUser.mustClick();
  
      TRI_IF_FAILURE("CollectionOperations::InsertDocumentWorker2") {
        THROW_ARANGO_EXCEPTION(TRI_ERROR_DEBUG);
      }
    }
    catch (...) {
      // an error occurred during insert
      // now revert the operation in all indexes 
      while (i > 0) {
        --i;
        try {
          indexes[i]->forget(collection, transaction, *mptr);
        }
        catch (...) {
        }
      }
      throw;
    }
  }

  // when we are here, the document is present in all indexes 
  
  TRI_IF_FAILURE("CollectionOperations::InsertDocumentWorker3") {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_DEBUG);
  }

  // now append the marker to the WAL
  WriteResult writeResult;
  {
    int res = WriteMarker(marker.get(), collection, writeResult);

    if (res != TRI_ERROR_NO_ERROR) {
      THROW_ARANGO_EXCEPTION(res);
    }
  }

  // make the master pointer point to the WAL location
  TRI_ASSERT_EXPENSIVE(writeResult.mem != nullptr);
  mptr->setDataPtr(writeResult.mem);

  // link the master pointer in the list of master pointers of the collection 
  mptr.link();

  TRI_ASSERT(mptr.get() != nullptr); 
  
  return OperationResult(mptr.get(), writeResult.tick, transaction->hasWaitForSync());
}

////////////////////////////////////////////////////////////////////////////////
/// @brief reads a document by its key
////////////////////////////////////////////////////////////////////////////////

OperationResult CollectionOperations::ReadDocumentWorker (TransactionScope* transactionScope,
                                                          TransactionCollection* collection,
                                                          IndexUser& indexUser,
                                                          Document const& document,
                                                          OperationOptions const& options) {
  if (document.key.empty()) {
    return OperationResult(TRI_ERROR_ARANGO_DOCUMENT_HANDLE_BAD);
  }
  
  auto primaryIndex = indexUser.primaryIndex();
 
  auto* transaction = transactionScope->transaction();
  auto mptr = primaryIndex->lookup(collection, transaction, document.key);

  if (mptr == nullptr) {
    return OperationResult(TRI_ERROR_ARANGO_DOCUMENT_NOT_FOUND);
  }

  TRI_ASSERT(mptr != nullptr);
    
  TRI_voc_rid_t actualRevision;
  int res = CheckRevision(mptr, document, options.policy, &actualRevision);

  if (res != TRI_ERROR_NO_ERROR) {
    // conflict etc.
    return OperationResult(res, actualRevision);
  }

  return OperationResult(mptr);  
}

////////////////////////////////////////////////////////////////////////////////
/// @brief remove a document
////////////////////////////////////////////////////////////////////////////////

OperationResult CollectionOperations::RemoveDocumentWorker (TransactionScope* transactionScope,
                                                            TransactionCollection* collection,
                                                            IndexUser& indexUser,
                                                            Document const& document,
                                                            OperationOptions const& options,
                                                            TransactionId& originalTransactionId) {
  auto* transaction = transactionScope->transaction();
  auto const transactionId = transaction->id();

  // remove document from primary index
  // this fetches the master pointer of the to-be-deleted revision
  // and sets its _to value to our transaction id

  auto primaryIndex = indexUser.primaryIndex();
  originalTransactionId.reset();

  // this throws on error
  auto mptr = primaryIndex->remove(collection, transaction, document.key, originalTransactionId);
  TRI_ASSERT(mptr != nullptr);
  indexUser.mustClick();

  TRI_voc_rid_t actualRevision;
  int res = CheckRevision(mptr, document, options.policy, &actualRevision);

  if (res != TRI_ERROR_NO_ERROR) {
    // conflict etc.
    // revert the value of the _to attribute
    if (! mptr->changeTo(transactionId, originalTransactionId)) {
      // oops, should not happen
      TRI_ASSERT(false);
    }

    return OperationResult(res, actualRevision);
  }

  TRI_voc_tick_t tick;
  try {
    // iterate over all secondary indexes while holding the index read-lock
    auto indexes = indexUser.indexes();

    for (size_t i = 1; i < indexes.size(); ++i) {
      // call remove for each secondary indexes
      indexes[i]->remove(collection, transaction, document.key, mptr);
    }
    
    // when we are here, the document has been removed from all indexes 

    // create a temporary remove marker on the heap
    // the marker memory will be freed automatically when we leave this method
    std::unique_ptr<triagens::wal::MvccRemoveMarker> marker(
      new triagens::wal::MvccRemoveMarker(transaction->vocbase()->_id,
                                          collection->id(),
                                          actualRevision,
                                          transaction->singleOperation() ? DefaultTransactionId : transactionId,
                                          document.key)
    );

    // now append the marker to the WAL
    WriteResult writeResult;
    res = WriteMarker(marker.get(), collection, writeResult);

    if (res != TRI_ERROR_NO_ERROR) {
      THROW_ARANGO_EXCEPTION(res);
    }

    tick = writeResult.tick;
  }
  catch (...) {
    // revert the value of the _to attribute
    if (! mptr->changeTo(transactionId, originalTransactionId)) {
      // oops, should not happen
      TRI_ASSERT(false);
    }

    // TODO: do we need to undo the remove() in the secondary indexes, too??
    throw;
  }

  TRI_ASSERT(mptr != nullptr);

  return OperationResult(mptr, tick, transaction->hasWaitForSync());
}

////////////////////////////////////////////////////////////////////////////////
/// @brief validate the revision of the document found 
////////////////////////////////////////////////////////////////////////////////
    
int CollectionOperations::CheckRevision (TRI_doc_mptr_t const* mptr,
                                         Document const& document,
                                         TRI_doc_update_policy_e policy,
                                         TRI_voc_rid_t* actualRevision) {
  if (actualRevision != nullptr) {
    *actualRevision = mptr->_rid;
  }

  if (document.revision == 0 ||
      document.revision == mptr->_rid) {
    // the caller did not specify a revision or
    // we found the expected revision or
    return TRI_ERROR_NO_ERROR;
  }

  if (policy == TRI_DOC_UPDATE_ERROR) {
    // revision mismatch
    return TRI_ERROR_ARANGO_CONFLICT;
  }

  if (policy == TRI_DOC_UPDATE_ONLY_IF_NEWER &&
      document.revision < mptr->_rid) {
    // currently only used by recovery. TODO: check if this condition makes sense now
    return TRI_ERROR_ARANGO_CONFLICT; 
  }

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a key if not specified, or validates it if specified
/// will throw if the key is invalid
////////////////////////////////////////////////////////////////////////////////
  
void CollectionOperations::CreateOrValidateKey (TransactionCollection* collection,
                                                Document* document) {                   
  if (! document->keySpecified) {
    TRI_ASSERT(document->key.empty());

    // no key specified. now let the collection create one
    document->key.assign(collection->generateKey(document->revision));

    if (document->key.empty()) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_OUT_OF_KEYS);
    }
  }
  else {
    // key was specified, now validate it
    collection->validateKey(document->key); 
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief append a marker to the WAL
////////////////////////////////////////////////////////////////////////////////

int CollectionOperations::WriteMarker (triagens::wal::Marker const* marker,
                                       TransactionCollection* collection,
                                       WriteResult& result) {
  auto logfileManager = triagens::wal::LogfileManager::instance();
  
  char* oldMarker = static_cast<char*>(marker->mem());
  auto genericMarker = reinterpret_cast<TRI_df_marker_t*>(oldMarker);

  if ((genericMarker->_type == TRI_WAL_MARKER_MVCC_DOCUMENT ||
       genericMarker->_type == TRI_WAL_MARKER_MVCC_EDGE) &&
      ! logfileManager->suppressShapeInformation()) {
    // In this case we have to take care of the legend, we know that the
    // marker does not have a legend so far, so first try to get away 
    // with this:
    // (Note that the latter also works for edges!
    auto oldm = reinterpret_cast<triagens::wal::mvcc_document_marker_t*>(oldMarker);
    TRI_voc_cid_t const cid   = oldm->_collectionId;
    TRI_shape_sid_t const sid = oldm->_shape;
    void* oldLegend;
    triagens::wal::SlotInfoCopy slotInfo = logfileManager->allocateAndWrite(oldMarker, marker->size(), false, cid, sid, 0, oldLegend);

    if (slotInfo.errorCode == TRI_ERROR_LEGEND_NOT_IN_WAL_FILE) {
      // Oh dear, we have to build a legend and patch the marker:
      triagens::basics::JsonLegend legend(collection->documentCollection()->getShaper());  // PROTECTED by trx in trxCollection

      int res = legend.addShape(sid, oldMarker + oldm->_offsetJson, oldm->_size - oldm->_offsetJson);

      if (res != TRI_ERROR_NO_ERROR) {
        return res;
      }

      int64_t sizeChanged = legend.getSize() - (oldm->_offsetJson - oldm->_offsetLegend);
      TRI_voc_size_t newMarkerSize = static_cast<TRI_voc_size_t>(oldm->_size + sizeChanged);

      // Now construct the new marker on the heap:
      char* newmarker = new char[newMarkerSize];
      memcpy(newmarker, oldMarker, oldm->_offsetLegend);
      try {
        legend.dump(newmarker + oldm->_offsetLegend);
      }
      catch (...) {
        delete[] newmarker;
        throw;
      }
      memcpy(newmarker + oldm->_offsetLegend + legend.getSize(), oldMarker + oldm->_offsetJson, oldm->_size - oldm->_offsetJson);

      // And fix its entries:
      auto newm = reinterpret_cast<triagens::wal::mvcc_document_marker_t*>(newmarker);
      newm->_size = newMarkerSize;
      newm->_offsetJson = (uint32_t) (oldm->_offsetLegend + legend.getSize());

      triagens::wal::SlotInfoCopy slotInfo2 = logfileManager->allocateAndWrite(newmarker, newMarkerSize, false, cid, sid, newm->_offsetLegend, oldLegend);
      delete[] newmarker;

      if (slotInfo2.errorCode != TRI_ERROR_NO_ERROR) {
        return slotInfo2.errorCode;
      }

      result.logfileId = slotInfo2.logfileId;
      result.mem       = slotInfo2.mem; 
      result.tick      = slotInfo2.tick;
    }
    else if (slotInfo.errorCode != TRI_ERROR_NO_ERROR) {
      return slotInfo.errorCode;
    }
    else {
      int64_t* legendPtr = reinterpret_cast<int64_t*>(oldMarker + oldm->_offsetLegend);
      *legendPtr =  reinterpret_cast<char*>(oldLegend) - reinterpret_cast<char*>(legendPtr);
      // This means that we can find the old legend relative to
      // the new position in the same WAL file.
      result.logfileId = slotInfo.logfileId;
      result.mem       = slotInfo.mem; 
      result.tick      = slotInfo.tick;
    }

  }
  else {  
    // No document or edge marker, just append it to the WAL:
    triagens::wal::SlotInfoCopy slotInfo = logfileManager->allocateAndWrite(marker->mem(), marker->size(), false);

    if (slotInfo.errorCode != TRI_ERROR_NO_ERROR) {
      // some error occurred
      return slotInfo.errorCode;
    }

    result.logfileId = slotInfo.logfileId;
    result.mem       = slotInfo.mem;
    result.tick      = slotInfo.tick;
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
