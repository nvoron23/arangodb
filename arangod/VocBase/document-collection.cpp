////////////////////////////////////////////////////////////////////////////////
/// @brief document collection with global read-write lock
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
/// @author Copyright 2014, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "document-collection.h"
#include "Basics/Barrier.h"
#include "Basics/conversions.h"
#include "Basics/files.h"
#include "Basics/logging.h"
#include "Basics/tri-strings.h"
#include "Basics/ThreadPool.h"
#include "Mvcc/CapConstraint.h"
#include "Mvcc/EdgeIndex.h"
#include "Mvcc/FulltextIndex.h"
#include "Mvcc/GeoIndex.h"
#include "Mvcc/HashIndex.h"
#include "Mvcc/Index.h"
#include "Mvcc/MasterpointerManager.h"
#include "Mvcc/OpenIterator.h"
#include "Mvcc/PrimaryIndex.h"
#include "Mvcc/SkiplistIndex.h"
#include "Mvcc/Transaction.h"
#include "Mvcc/TransactionCollection.h"
#include "Mvcc/TransactionScope.h"
#include "RestServer/ArangoServer.h"
#include "ShapedJson/shape-accessor.h"
#include "Utils/Exception.h"
#include "VocBase/barrier.h"
#include "VocBase/key-generator.h"
#include "VocBase/server.h"
#include "VocBase/update-policy.h"
#include "VocBase/voc-shaper.h"
#include "Wal/LogfileManager.h"
#include "Wal/Marker.h"
#include "Wal/Slots.h"

using namespace triagens::arango;

////////////////////////////////////////////////////////////////////////////////
/// @brief return a pointer to the beginning of the marker
////////////////////////////////////////////////////////////////////////////////

#ifdef TRI_ENABLE_MAINTAINER_MODE
void const* TRI_doc_mptr_t::getDataPtr () const {
  TransactionBase::assertCurrentTrxActive();
  return _dataptr.load(std::memory_order_acquire);
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief set the pointer to the beginning of the memory for the marker
////////////////////////////////////////////////////////////////////////////////

#ifdef TRI_ENABLE_MAINTAINER_MODE
void TRI_doc_mptr_t::setDataPtr (void const* d) {
  TransactionBase::assertCurrentTrxActive();
  _dataptr.store(d, std::memory_order_release);
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief return a pointer to the beginning of the marker, copy object
////////////////////////////////////////////////////////////////////////////////

#ifdef TRI_ENABLE_MAINTAINER_MODE
void const* TRI_doc_mptr_copy_t::getDataPtr () const {
  TransactionBase::assertSomeTrxInScope();
  return _dataptr;
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief set the pointer to the beginning of the memory for the marker,
/// copy object
////////////////////////////////////////////////////////////////////////////////

#ifdef TRI_ENABLE_MAINTAINER_MODE
void TRI_doc_mptr_copy_t::setDataPtr (void const* d) {
  TransactionBase::assertSomeTrxInScope();
  _dataptr = d;
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief create a document collection
////////////////////////////////////////////////////////////////////////////////

TRI_document_collection_t::TRI_document_collection_t () 
  : _useSecondaryIndexes(true),
    _indexesLock(),
    _indexes(),
    _documentCounterLock(),
    _documentCount(0),
    _documentSize(0),
    _revisionId(0),
    _masterpointerManager(nullptr),
    _primaryIndex(nullptr),
    _keyGenerator(nullptr),
    _uncollectedLogfileEntries(0) {

  _tickMax = 0;

  _masterpointerManager = new triagens::mvcc::MasterpointerManager;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy a document collection
////////////////////////////////////////////////////////////////////////////////

TRI_document_collection_t::~TRI_document_collection_t () {
  delete _keyGenerator;

  shutdownIndexes();

  delete _masterpointerManager;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return a pointer to the shaper
////////////////////////////////////////////////////////////////////////////////

#ifdef TRI_ENABLE_MAINTAINER_MODE
TRI_shaper_t* TRI_document_collection_t::getShaper () const {
  if (! TRI_ContainsBarrierList(&_barrierList, TRI_BARRIER_ELEMENT)) {
    TransactionBase::assertSomeTrxInScope();
  }
  return _shaper;
}
#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief constructs an index file name based on the collection directory name
////////////////////////////////////////////////////////////////////////////////

std::string TRI_document_collection_t::buildIndexFilename (TRI_idx_iid_t iid) const {
  std::string filename;
  filename.append(_directory);
  filename.append(TRI_DIR_SEPARATOR_STR);
  filename.append("index-");
  filename.append(std::to_string(iid));
  filename.append(".json");

  return filename;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief shut down indexes
////////////////////////////////////////////////////////////////////////////////

void TRI_document_collection_t::shutdownIndexes () {
  WRITE_LOCKER(_indexesLock);

  for (auto it: _indexes) {
    delete it;
  }
  _indexes.clear();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief adds an index to the collection
////////////////////////////////////////////////////////////////////////////////

triagens::mvcc::Index* TRI_document_collection_t::addIndex (triagens::mvcc::Index* index) {
  WRITE_LOCKER(_indexesLock);

  if (! _indexes.empty() && _indexes.back()->type() == triagens::mvcc::Index::TRI_IDX_TYPE_CAP_CONSTRAINT) {
    // we have a cap constraint as the last present index in the vector
    // make sure the cap constraint stays at the end of the list after our insertion
    _indexes.emplace_back(_indexes.back());

    TRI_ASSERT(_indexes.size() >= 2);
    _indexes[_indexes.size() - 2] = index;

    TRI_ASSERT(_indexes.back()->type() == triagens::mvcc::Index::TRI_IDX_TYPE_CAP_CONSTRAINT);
  }
  else {
    _indexes.emplace_back(index);

    if (index->type() == triagens::mvcc::Index::TRI_IDX_TYPE_PRIMARY_INDEX) {
      TRI_ASSERT(_primaryIndex == nullptr);
      _primaryIndex = static_cast<triagens::mvcc::PrimaryIndex*>(index);
    }
  }

  return index;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the list of indexes of the collection, without holding the
/// lock on the indexes
////////////////////////////////////////////////////////////////////////////////
  
std::vector<triagens::mvcc::Index*> TRI_document_collection_t::indexes () const {
  return _indexes;
} 

////////////////////////////////////////////////////////////////////////////////
/// @brief lookup an index by id
////////////////////////////////////////////////////////////////////////////////
  
triagens::mvcc::Index* TRI_document_collection_t::lookupIndex (TRI_idx_iid_t id) {
  READ_LOCKER(_indexesLock);

  for (auto idx : _indexes) {
    if (idx->id()== id) {
      return idx;
    }
  }

  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief unlinks an index from the list of indexes, but does not delete the
/// index itself
/// note: the caller must have acquired the write-lock on the indexes of the
/// collection
////////////////////////////////////////////////////////////////////////////////

bool TRI_document_collection_t::unlinkIndex (triagens::mvcc::Index* index) {
  if (index == nullptr || 
      index->type() == triagens::mvcc::Index::TRI_IDX_TYPE_PRIMARY_INDEX ||
      index->type() == triagens::mvcc::Index::TRI_IDX_TYPE_EDGE_INDEX) {
    // invalid index id or primary index
    return false;
  }

  size_t i = 0;
  for (auto it : _indexes) {
    if (it == index) {
      _indexes.erase(_indexes.begin() + i);
      return true;
    }
    ++i;
  }

  return false;
}
 
////////////////////////////////////////////////////////////////////////////////
/// @brief drops an index, including index file removal and replication
////////////////////////////////////////////////////////////////////////////////

bool TRI_document_collection_t::dropIndex (triagens::mvcc::Index* index,
                                           bool writeMarker) {
  int res = removeIndexFile(index->id());

  if (writeMarker) {
    try {
      triagens::wal::DropIndexMarker marker(_vocbase->_id, _info._cid, index->id());
      triagens::wal::SlotInfoCopy slotInfo = triagens::wal::LogfileManager::instance()->allocateAndWrite(marker, false);

      if (slotInfo.errorCode != TRI_ERROR_NO_ERROR) {
        THROW_ARANGO_EXCEPTION(slotInfo.errorCode);
      }
    }
    catch (triagens::arango::Exception const& ex) {
      res = ex.code();
    }
    catch (...) {
      res = TRI_ERROR_INTERNAL;
    }

    LOG_WARNING("could not save index drop marker in log: %s", TRI_errno_string(res));
  }
  
  delete index;

  // TODO: what to do here if removal failed?
  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief get index statistics
////////////////////////////////////////////////////////////////////////////////
  
void TRI_document_collection_t::indexStats (int64_t& totalSize,
                                            TRI_voc_ssize_t& count) {
  totalSize = 0;

  READ_LOCKER(_indexesLock);
 
  count = static_cast<TRI_voc_ssize_t>(_indexes.size());

  for (auto idx : _indexes) {
    totalSize += static_cast<int64_t>(idx->memory());
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief lookup an index by type
////////////////////////////////////////////////////////////////////////////////
  
triagens::mvcc::Index* TRI_document_collection_t::lookupIndex (triagens::mvcc::Index::IndexType type) {
  READ_LOCKER(_indexesLock);

  for (auto idx : _indexes) {
    if (idx->type()== type) {
      return idx;
    }
  }

  return nullptr;
} 

////////////////////////////////////////////////////////////////////////////////
/// @brief read-lock the list of indexes
////////////////////////////////////////////////////////////////////////////////

void TRI_document_collection_t::readLockIndexes () {
  _indexesLock.readLock();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief read-unlock the list of indexes
////////////////////////////////////////////////////////////////////////////////

void TRI_document_collection_t::readUnlockIndexes () {
  _indexesLock.readUnlock();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief write-lock the list of indexes
////////////////////////////////////////////////////////////////////////////////

void TRI_document_collection_t::writeLockIndexes () {
  _indexesLock.writeLock();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief write-unlock the list of indexes
////////////////////////////////////////////////////////////////////////////////

void TRI_document_collection_t::writeUnlockIndexes () {
  _indexesLock.writeUnlock();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return a pointer to the collection's primary index
////////////////////////////////////////////////////////////////////////////////

triagens::mvcc::PrimaryIndex* TRI_document_collection_t::primaryIndex () {
  return _primaryIndex;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief removes an index file
////////////////////////////////////////////////////////////////////////////////

int TRI_document_collection_t::removeIndexFile (TRI_idx_iid_t iid) {
  std::string&& filename = buildIndexFilename(iid);

  int res = TRI_UnlinkFile(filename.c_str());

  if (res != TRI_ERROR_NO_ERROR) {
    LOG_ERROR("cannot remove index definition: %s", TRI_last_error());
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief saves an index
////////////////////////////////////////////////////////////////////////////////

int TRI_document_collection_t::saveIndexFile (triagens::mvcc::Index const* index,
                                              bool writeMarker) {
  // convert into JSON
  auto json = index->toJson(TRI_UNKNOWN_MEM_ZONE);

  if (json.json() == nullptr) {
    LOG_TRACE("cannot save index definition: index cannot be jsonified");
    return TRI_set_errno(TRI_ERROR_INTERNAL);
  }

  std::string&& filename = buildIndexFilename(index->id());

  // and save
  bool ok = TRI_SaveJson(filename.c_str(), json.json(), _vocbase->_settings.forceSyncProperties);

  if (! ok) {
    LOG_ERROR("cannot save index definition: %s", TRI_last_error());
    return TRI_errno();
  }

  int res = TRI_ERROR_NO_ERROR;

  if (writeMarker) {
    try {
      triagens::wal::CreateIndexMarker marker(_vocbase->_id, _info._cid, index->id(), triagens::basics::JsonHelper::toString(json.json()));
      triagens::wal::SlotInfoCopy slotInfo = triagens::wal::LogfileManager::instance()->allocateAndWrite(marker, false);

      if (slotInfo.errorCode != TRI_ERROR_NO_ERROR) {
        THROW_ARANGO_EXCEPTION(slotInfo.errorCode);
      }
    }
    catch (triagens::arango::Exception const& ex) {
      res = ex.code();
    }
    catch (...) {
      res = TRI_ERROR_INTERNAL;
    }
  }

  // TODO: what to do here in case of error?
  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the number of documents in the collection
////////////////////////////////////////////////////////////////////////////////

int64_t TRI_document_collection_t::documentCount () {
  READ_LOCKER(_documentCounterLock);
  return _documentCount;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief update the number of documents in the collection
////////////////////////////////////////////////////////////////////////////////

void TRI_document_collection_t::updateDocumentCount (int64_t value) {
  if (value == 0) {
    return;
  }

  WRITE_LOCKER(_documentCounterLock);
  TRI_ASSERT_EXPENSIVE(_documentCount + value >= 0);
  _documentCount += value;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the size of documents in the collection
////////////////////////////////////////////////////////////////////////////////

int64_t TRI_document_collection_t::documentSize () {
  READ_LOCKER(_documentCounterLock);
  return _documentSize;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief update the size of documents in the collection
////////////////////////////////////////////////////////////////////////////////

void TRI_document_collection_t::updateDocumentSize (int64_t value) {
  if (value == 0) {
    return;
  }

  WRITE_LOCKER(_documentCounterLock);
  _documentSize += value;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief update the statistics of the collection
////////////////////////////////////////////////////////////////////////////////

void TRI_document_collection_t::updateDocumentStats (int64_t count,
                                                     int64_t size) {
  if (count == 0 && size == 0) {
    return;
  }

  WRITE_LOCKER(_documentCounterLock);
  _documentCount += count;
  _documentSize  += size;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the last revision id in the collection
////////////////////////////////////////////////////////////////////////////////

TRI_voc_rid_t TRI_document_collection_t::revisionId () {
  READ_LOCKER(_documentCounterLock);
  return _revisionId;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief update the last revision id in the collection
////////////////////////////////////////////////////////////////////////////////

void TRI_document_collection_t::updateRevisionId (TRI_voc_rid_t revisionId) {
  WRITE_LOCKER(_documentCounterLock);
  if (revisionId > _revisionId) {
    _revisionId = revisionId;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the collection's master pointer manager
////////////////////////////////////////////////////////////////////////////////

triagens::mvcc::MasterpointerManager* TRI_document_collection_t::masterpointerManager () const {
  return _masterpointerManager;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief fill an index
////////////////////////////////////////////////////////////////////////////////

void TRI_document_collection_t::fillIndex (triagens::mvcc::Index* index) {
  size_t const numDocuments = documentCount();
  index->sizeHint(numDocuments);

  auto primaryIndex = lookupIndex(triagens::mvcc::Index::TRI_IDX_TYPE_PRIMARY_INDEX);

  if (primaryIndex == nullptr) {
    return;
  }
  
  static_cast<triagens::mvcc::PrimaryIndex*>(primaryIndex)->iterate([&index](TRI_doc_mptr_t* document) -> void {
    index->insert(document);
  });
}

// -----------------------------------------------------------------------------
// --SECTION--                                              forward declarations
// -----------------------------------------------------------------------------

static int CapConstraintFromJson (TRI_document_collection_t*,
                                  TRI_json_t const*,
                                  TRI_idx_iid_t,
                                  triagens::mvcc::Index*&);

static int GeoIndexFromJson (TRI_document_collection_t*,
                             TRI_json_t const*,
                             TRI_idx_iid_t,
                             triagens::mvcc::Index*&);

static int HashIndexFromJson (TRI_document_collection_t*,
                              TRI_json_t const*,
                              TRI_idx_iid_t,
                              triagens::mvcc::Index*&);

static int SkiplistIndexFromJson (TRI_document_collection_t*,
                                  TRI_json_t const*,
                                  TRI_idx_iid_t,
                                  triagens::mvcc::Index*&);

static int FulltextIndexFromJson (TRI_document_collection_t*,
                                  TRI_json_t const*,
                                  TRI_idx_iid_t,
                                  triagens::mvcc::Index*&);

// -----------------------------------------------------------------------------
// --SECTION--                                                  HELPER FUNCTIONS
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                 private functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief hashes a datafile identifier
////////////////////////////////////////////////////////////////////////////////

static uint64_t HashKeyDatafile (TRI_associative_pointer_t* array, void const* key) {
  return *(static_cast<TRI_voc_fid_t const*>(key));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief hashes a datafile identifier
////////////////////////////////////////////////////////////////////////////////

static uint64_t HashElementDatafile (TRI_associative_pointer_t* array, void const* element) {
  TRI_doc_datafile_info_t const* e = static_cast<TRI_doc_datafile_info_t const*>(element);

  return e->_fid;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief compares a datafile identifier and a datafile info
////////////////////////////////////////////////////////////////////////////////

static bool IsEqualKeyElementDatafile (TRI_associative_pointer_t* array, void const* key, void const* element) {
  TRI_voc_fid_t const* k = static_cast<TRI_voc_fid_t const*>(key);
  TRI_doc_datafile_info_t const* e = static_cast<TRI_doc_datafile_info_t const*>(element);

  return *k == e->_fid;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief free an assoc array of datafile infos
////////////////////////////////////////////////////////////////////////////////

static void FreeDatafileInfo (TRI_doc_datafile_info_t* dfi) {
  TRI_Free(TRI_UNKNOWN_MEM_ZONE, dfi);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief size of a primary collection
///
/// the caller must have read-locked the collection!
////////////////////////////////////////////////////////////////////////////////

static TRI_voc_size_t Count (TRI_document_collection_t* document) {
  return (TRI_voc_size_t) document->_numberDocuments;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief set the collection tick with the marker's tick value
////////////////////////////////////////////////////////////////////////////////

static inline void SetRevision (TRI_document_collection_t* document,
                                TRI_voc_rid_t rid,
                                bool force) {
  TRI_col_info_t* info = &document->_info;

  if (force || rid > info->_revision) {
    info->_revision = rid;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief ensures that an error code is set in all required places
////////////////////////////////////////////////////////////////////////////////

static void EnsureErrorCode (int code) {
  if (code == TRI_ERROR_NO_ERROR) {
    // must have an error code
    code = TRI_ERROR_INTERNAL;
  }

  TRI_set_errno(code);
  errno = code;
}

// -----------------------------------------------------------------------------
// --SECTION--                                               DOCUMENT COLLECTION
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                 private functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief generates a new revision id if not yet set
////////////////////////////////////////////////////////////////////////////////

static inline TRI_voc_rid_t GetRevisionId (TRI_voc_rid_t previous) {
  if (previous != 0) {
    return previous;
  }

  // generate new revision id
  return static_cast<TRI_voc_rid_t>(TRI_NewTickServer());
}

////////////////////////////////////////////////////////////////////////////////
/// @brief read locks a collection
////////////////////////////////////////////////////////////////////////////////

static int BeginRead (TRI_document_collection_t* document) {
  if (triagens::arango::Transaction::_makeNolockHeaders != nullptr) {
    std::string collName(document->_info._name);
    auto it = triagens::arango::Transaction::_makeNolockHeaders->find(collName);
    if (it != triagens::arango::Transaction::_makeNolockHeaders->end()) {
      // do not lock by command
      // LOCKING-DEBUG
      // std::cout << "BeginRead blocked: " << document->_info._name << std::endl;
      return TRI_ERROR_NO_ERROR;
    }
  }
  // LOCKING-DEBUG
  // std::cout << "BeginRead: " << document->_info._name << std::endl;
  TRI_READ_LOCK_DOCUMENTS_INDEXES_PRIMARY_COLLECTION(document);

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief read unlocks a collection
////////////////////////////////////////////////////////////////////////////////

static int EndRead (TRI_document_collection_t* document) {
  if (triagens::arango::Transaction::_makeNolockHeaders != nullptr) {
    std::string collName(document->_info._name);
    auto it = triagens::arango::Transaction::_makeNolockHeaders->find(collName);
    if (it != triagens::arango::Transaction::_makeNolockHeaders->end()) {
      // do not lock by command
      // LOCKING-DEBUG
      // std::cout << "EndRead blocked: " << document->_info._name << std::endl;
      return TRI_ERROR_NO_ERROR;
    }
  }
  // LOCKING-DEBUG
  // std::cout << "EndRead: " << document->_info._name << std::endl;
  TRI_READ_UNLOCK_DOCUMENTS_INDEXES_PRIMARY_COLLECTION(document);

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief write locks a collection
////////////////////////////////////////////////////////////////////////////////

static int BeginWrite (TRI_document_collection_t* document) {
  if (triagens::arango::Transaction::_makeNolockHeaders != nullptr) {
    std::string collName(document->_info._name);
    auto it = triagens::arango::Transaction::_makeNolockHeaders->find(collName);
    if (it != triagens::arango::Transaction::_makeNolockHeaders->end()) {
      // do not lock by command
      // LOCKING-DEBUG
      // std::cout << "BeginWrite blocked: " << document->_info._name << std::endl;
      return TRI_ERROR_NO_ERROR;
    }
  }
  // LOCKING_DEBUG
  // std::cout << "BeginWrite: " << document->_info._name << std::endl;
  TRI_WRITE_LOCK_DOCUMENTS_INDEXES_PRIMARY_COLLECTION(document);

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief write unlocks a collection
////////////////////////////////////////////////////////////////////////////////

static int EndWrite (TRI_document_collection_t* document) {
  if (triagens::arango::Transaction::_makeNolockHeaders != nullptr) {
    std::string collName(document->_info._name);
    auto it = triagens::arango::Transaction::_makeNolockHeaders->find(collName);
    if (it != triagens::arango::Transaction::_makeNolockHeaders->end()) {
      // do not lock by command
      // LOCKING-DEBUG
      // std::cout << "EndWrite blocked: " << document->_info._name << std::endl;
      return TRI_ERROR_NO_ERROR;
    }
  }
  // LOCKING-DEBUG
  // std::cout << "EndWrite: " << document->_info._name << std::endl;
  TRI_WRITE_UNLOCK_DOCUMENTS_INDEXES_PRIMARY_COLLECTION(document);

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief read locks a collection, with a timeout (in µseconds)
////////////////////////////////////////////////////////////////////////////////

static int BeginReadTimed (TRI_document_collection_t* document,
                           uint64_t timeout,
                           uint64_t sleepPeriod) {
  if (triagens::arango::Transaction::_makeNolockHeaders != nullptr) {
    std::string collName(document->_info._name);
    auto it = triagens::arango::Transaction::_makeNolockHeaders->find(collName);
    if (it != triagens::arango::Transaction::_makeNolockHeaders->end()) {
      // do not lock by command
      // LOCKING-DEBUG
      // std::cout << "BeginReadTimed blocked: " << document->_info._name << std::endl;
      return TRI_ERROR_NO_ERROR;
    }
  }
  uint64_t waited = 0;

  // LOCKING-DEBUG
  // std::cout << "BeginReadTimed: " << document->_info._name << std::endl;
  while (! TRI_TRY_READ_LOCK_DOCUMENTS_INDEXES_PRIMARY_COLLECTION(document)) {
#ifdef _WIN32
    usleep((unsigned long) sleepPeriod);
#else
    usleep((useconds_t) sleepPeriod);
#endif

    waited += sleepPeriod;

    if (waited > timeout) {
      return TRI_ERROR_LOCK_TIMEOUT;
    }
  }

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief write locks a collection, with a timeout
////////////////////////////////////////////////////////////////////////////////

static int BeginWriteTimed (TRI_document_collection_t* document,
                            uint64_t timeout,
                            uint64_t sleepPeriod) {
  if (triagens::arango::Transaction::_makeNolockHeaders != nullptr) {
    std::string collName(document->_info._name);
    auto it = triagens::arango::Transaction::_makeNolockHeaders->find(collName);
    if (it != triagens::arango::Transaction::_makeNolockHeaders->end()) {
      // do not lock by command
      // LOCKING-DEBUG
      // std::cout << "BeginWriteTimed blocked: " << document->_info._name << std::endl;
      return TRI_ERROR_NO_ERROR;
    }
  }
  uint64_t waited = 0;

  // LOCKING-DEBUG
  // std::cout << "BeginWriteTimed: " << document->_info._name << std::endl;
  while (! TRI_TRY_WRITE_LOCK_DOCUMENTS_INDEXES_PRIMARY_COLLECTION(document)) {
#ifdef _WIN32
    usleep((unsigned long) sleepPeriod);
#else
    usleep((useconds_t) sleepPeriod);
#endif

    waited += sleepPeriod;

    if (waited > timeout) {
      return TRI_ERROR_LOCK_TIMEOUT;
    }
  }

  return TRI_ERROR_NO_ERROR;
}

// -----------------------------------------------------------------------------
// --SECTION--                                               DOCUMENT COLLECTION
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                 private functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief iterator for index open
////////////////////////////////////////////////////////////////////////////////

static bool OpenIndexIterator (char const* filename,
                               void* data) {
  // load json description of the index
  std::unique_ptr<TRI_json_t> json(TRI_JsonFile(TRI_UNKNOWN_MEM_ZONE, filename, nullptr));

  // json must be a index description
  if (! TRI_IsObjectJson(json.get())) {
    LOG_ERROR("cannot read index definition from '%s'", filename);

    return false;
  }

  triagens::mvcc::Index* index;
  int res = TRI_FromJsonIndexDocumentCollection(static_cast<TRI_document_collection_t*>(data), json.get(), index);

  if (res != TRI_ERROR_NO_ERROR) {
    // error was already printed if we get here
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief returns information about the collection
/// note: the collection lock must be held when calling this function
////////////////////////////////////////////////////////////////////////////////

static TRI_doc_collection_info_t* Figures (TRI_document_collection_t* document) {
  // prefill with 0's to init counters
  TRI_doc_collection_info_t* info = static_cast<TRI_doc_collection_info_t*>(TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, sizeof(TRI_doc_collection_info_t), true));

  if (info == nullptr) {
    return nullptr;
  }

  for (size_t i = 0;  i < document->_datafileInfo._nrAlloc;  ++i) {
    auto d = static_cast<TRI_doc_datafile_info_t const*>(document->_datafileInfo._table[i]);

    if (d != nullptr) {
      info->_numberAlive        += d->_numberAlive;
      info->_numberDead         += d->_numberDead;
      info->_numberDeletion     += d->_numberDeletion;
      info->_numberShapes       += d->_numberShapes;
      info->_numberAttributes   += d->_numberAttributes;
      info->_numberTransactions += d->_numberTransactions;

      info->_sizeAlive          += d->_sizeAlive;
      info->_sizeDead           += d->_sizeDead;
      info->_sizeShapes         += d->_sizeShapes;
      info->_sizeAttributes     += d->_sizeAttributes;
      info->_sizeTransactions   += d->_sizeTransactions;
    }
  }

  // add the file sizes for datafiles and journals
  TRI_collection_t* base = document;

  for (size_t i = 0; i < base->_datafiles._length; ++i) {
    auto df = static_cast<TRI_datafile_t const*>(base->_datafiles._buffer[i]);

    info->_datafileSize += (int64_t) df->_maximalSize;
    ++info->_numberDatafiles;
  }

  for (size_t i = 0; i < base->_journals._length; ++i) {
    auto df = static_cast<TRI_datafile_t const*>(base->_journals._buffer[i]);

    info->_journalfileSize += (int64_t) df->_maximalSize;
    ++info->_numberJournalfiles;
  }

  for (size_t i = 0; i < base->_compactors._length; ++i) {
    auto df = static_cast<TRI_datafile_t const*>(base->_compactors._buffer[i]);

    info->_compactorfileSize += (int64_t) df->_maximalSize;
    ++info->_numberCompactorfiles;
  }

  // add index information
  document->indexStats(info->_sizeIndexes, info->_numberIndexes);

  // get information about shape files (DEPRECATED, thus hard-coded to 0)
  info->_shapefileSize    = 0;
  info->_numberShapefiles = 0;

  info->_uncollectedLogfileEntries = document->_uncollectedLogfileEntries;
  info->_tickMax = document->_tickMax;

  return info;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief initialises a document collection
////////////////////////////////////////////////////////////////////////////////

static int InitBaseDocumentCollection (TRI_document_collection_t* document,
                                       TRI_shaper_t* shaper) {
  document->setShaper(shaper);
  document->_numberDocuments    = 0;
  document->_lastCompaction     = 0.0;

  document->size                = Count;


  int res = TRI_InitAssociativePointer(&document->_datafileInfo,
                                       TRI_UNKNOWN_MEM_ZONE,
                                       HashKeyDatafile,
                                       HashElementDatafile,
                                       IsEqualKeyElementDatafile,
                                       nullptr);

  if (res != TRI_ERROR_NO_ERROR) {
    return res;
  }

  TRI_InitBarrierList(&document->_barrierList, document);

  TRI_InitReadWriteLock(&document->_lock);
  TRI_InitReadWriteLock(&document->_compactionLock);

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroys a primary collection
////////////////////////////////////////////////////////////////////////////////

static void DestroyBaseDocumentCollection (TRI_document_collection_t* document) {
  if (document->_keyGenerator != nullptr) {
    delete document->_keyGenerator;
    document->_keyGenerator = nullptr;
  }

  TRI_DestroyReadWriteLock(&document->_compactionLock);
  TRI_DestroyReadWriteLock(&document->_lock);

  {
    TransactionBase trx(true);  // just to protect the following call
    if (document->getShaper() != nullptr) {  // PROTECTED by trx here
      TRI_FreeVocShaper(document->getShaper());  // PROTECTED by trx here
    }
  }

  size_t const n = document->_datafileInfo._nrAlloc;

  for (size_t i = 0; i < n; ++i) {
    TRI_doc_datafile_info_t* dfi = static_cast<TRI_doc_datafile_info_t*>(document->_datafileInfo._table[i]);

    if (dfi != nullptr) {
      FreeDatafileInfo(dfi);
    }
  }

  TRI_DestroyAssociativePointer(&document->_datafileInfo);
  TRI_DestroyBarrierList(&document->_barrierList);
  TRI_DestroyCollection(document);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief initialises a document collection
////////////////////////////////////////////////////////////////////////////////

static bool InitDocumentCollection (TRI_document_collection_t* document,
                                    TRI_shaper_t* shaper) {
  document->_uncollectedLogfileEntries = 0;

  int res = InitBaseDocumentCollection(document, shaper);

  if (res != TRI_ERROR_NO_ERROR) {
    TRI_DestroyCollection(document);
    TRI_set_errno(res);

    return false;
  }

  // add an MVCC primary index to the collection
  auto primaryIndex = new triagens::mvcc::PrimaryIndex(0, document);
  try {
    document->addIndex(primaryIndex);
  }
  catch (...) {
    delete primaryIndex;
    DestroyBaseDocumentCollection(document);
    TRI_set_errno(TRI_ERROR_OUT_OF_MEMORY);

    return false;
  }

  // create edges index
  if (document->_info._type == TRI_COL_TYPE_EDGE) {
    // add an MVCC edge index to the collection
    auto edgeIndex = new triagens::mvcc::EdgeIndex(document->_info._cid, document);
    try {
      document->addIndex(edgeIndex);
    }
    catch (...) {
      delete edgeIndex;
      DestroyBaseDocumentCollection(document);
      TRI_set_errno(TRI_ERROR_OUT_OF_MEMORY);

      return false;
    }
  }

  TRI_InitCondition(&document->_journalsCondition);

  // setup methods
  document->beginRead         = BeginRead;
  document->endRead           = EndRead;

  document->beginWrite        = BeginWrite;
  document->endWrite          = EndWrite;

  document->beginReadTimed    = BeginReadTimed;
  document->beginWriteTimed   = BeginWriteTimed;

  document->figures           = Figures;

  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief iterate all markers of the collection
////////////////////////////////////////////////////////////////////////////////

static int IterateMarkersCollection (TRI_collection_t* collection) {
  triagens::mvcc::OpenIteratorState state(reinterpret_cast<TRI_document_collection_t*>(collection));
  TRI_IterateCollection(collection, triagens::mvcc::OpenIterator::execute, &state);
  
  LOG_TRACE("found %llu document markers, %llu deletion markers for collection '%s'",
            (unsigned long long) state.insertions,
            (unsigned long long) state.deletions,
            collection->_info._name);

  return TRI_ERROR_NO_ERROR;
}

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a new collection
////////////////////////////////////////////////////////////////////////////////

TRI_document_collection_t* TRI_CreateDocumentCollection (TRI_vocbase_t* vocbase,
                                                         char const* path,
                                                         TRI_col_info_t* parameters,
                                                         TRI_voc_cid_t cid) {
  if (cid > 0) {
    TRI_UpdateTickServer(cid);
  }
  else {
    cid = TRI_NewTickServer();
  }

  parameters->_cid = cid;

  // check if we can generate the key generator
  KeyGenerator* keyGenerator = KeyGenerator::factory(parameters->_keyOptions);

  if (keyGenerator == nullptr) {
    TRI_set_errno(TRI_ERROR_ARANGO_INVALID_KEY_GENERATOR);
    return nullptr;
  }

  // first create the document collection
  TRI_document_collection_t* document;
  try {
    document = new TRI_document_collection_t();
  }
  catch (std::exception&) {
    document = nullptr;
  }

  if (document == nullptr) {
    delete keyGenerator;
    LOG_WARNING("cannot create document collection");
    TRI_set_errno(TRI_ERROR_OUT_OF_MEMORY);

    return nullptr;
  }

  document->_keyGenerator = keyGenerator;

  TRI_collection_t* collection = TRI_CreateCollection(vocbase, document, path, parameters);

  if (collection == nullptr) {
    delete document;
    LOG_ERROR("cannot create document collection");

    return nullptr;
  }

  TRI_shaper_t* shaper = TRI_CreateVocShaper(vocbase, document);

  if (shaper == nullptr) {
    LOG_ERROR("cannot create shaper");

    TRI_CloseCollection(collection);
    TRI_DestroyCollection(collection);
    delete document;
    return nullptr;
  }

  // create document collection and shaper
  if (false == InitDocumentCollection(document, shaper)) {
    LOG_ERROR("cannot initialise document collection");

    TRI_CloseCollection(collection);
    TRI_DestroyCollection(collection);
    delete document;
    return nullptr;
  }

  document->_keyGenerator = keyGenerator;

  // save the parameters block (within create, no need to lock)
  bool doSync = vocbase->_settings.forceSyncProperties;
  int res = TRI_SaveCollectionInfo(collection->_directory, parameters, doSync);

  if (res != TRI_ERROR_NO_ERROR) {
    LOG_ERROR("cannot save collection parameters in directory '%s': '%s'",
              collection->_directory,
              TRI_last_error());

    TRI_CloseCollection(collection);
    TRI_DestroyCollection(collection);
    delete document;
    return nullptr;
  }

  TransactionBase trx(true);  // just to protect the following call
  TRI_ASSERT(document->getShaper() != nullptr);  // ONLY IN COLLECTION CREATION, PROTECTED by trx here

  return document;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief frees the memory allocated, but does not free the pointer
///
/// Note that the collection must be closed first.
////////////////////////////////////////////////////////////////////////////////

void TRI_DestroyDocumentCollection (TRI_document_collection_t* document) {
  TRI_DestroyCondition(&document->_journalsCondition);
  DestroyBaseDocumentCollection(document);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief frees the memory allocated and frees the pointer
////////////////////////////////////////////////////////////////////////////////

void TRI_FreeDocumentCollection (TRI_document_collection_t* document) {
  TRI_DestroyDocumentCollection(document);
  delete document;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief removes a datafile description
////////////////////////////////////////////////////////////////////////////////

void TRI_RemoveDatafileInfoDocumentCollection (TRI_document_collection_t* document,
                                              TRI_voc_fid_t fid) {
  TRI_RemoveKeyAssociativePointer(&document->_datafileInfo, &fid);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief finds a datafile description
////////////////////////////////////////////////////////////////////////////////

TRI_doc_datafile_info_t* TRI_FindDatafileInfoDocumentCollection (TRI_document_collection_t* document,
                                                                TRI_voc_fid_t fid,
                                                                bool create) {
  TRI_doc_datafile_info_t const* found = static_cast<TRI_doc_datafile_info_t const*>(TRI_LookupByKeyAssociativePointer(&document->_datafileInfo, &fid));

  if (found != nullptr) {
    return const_cast<TRI_doc_datafile_info_t*>(found);
  }

  if (! create) {
    return nullptr;
  }

  // allocate and set to 0
  TRI_doc_datafile_info_t* dfi = static_cast<TRI_doc_datafile_info_t*>(TRI_Allocate(TRI_UNKNOWN_MEM_ZONE, sizeof(TRI_doc_datafile_info_t), true));

  if (dfi == nullptr) {
    return nullptr;
  }

  dfi->_fid = fid;

  TRI_InsertKeyAssociativePointer(&document->_datafileInfo, &fid, dfi, true);

  return dfi;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a journal
///
/// Note that the caller must hold a lock protecting the _journals entry.
////////////////////////////////////////////////////////////////////////////////

TRI_datafile_t* TRI_CreateDatafileDocumentCollection (TRI_document_collection_t* document,
                                                      TRI_voc_fid_t fid,
                                                      TRI_voc_size_t journalSize,
                                                      bool isCompactor) {
  TRI_ASSERT(fid > 0);

  TRI_datafile_t* journal;

  if (document->_info._isVolatile) {
    // in-memory collection
    journal = TRI_CreateDatafile(nullptr, fid, journalSize, true);
  }
  else {
    // construct a suitable filename (which may be temporary at the beginning)
    char* number = TRI_StringUInt64(fid);
    char* jname;
    if (isCompactor) {
      jname = TRI_Concatenate3String("compaction-", number, ".db");
    }
    else {
      jname = TRI_Concatenate3String("temp-", number, ".db");
    }

    char* filename = TRI_Concatenate2File(document->_directory, jname);

    TRI_FreeString(TRI_CORE_MEM_ZONE, number);
    TRI_FreeString(TRI_CORE_MEM_ZONE, jname);

    TRI_IF_FAILURE("CreateJournalDocumentCollection") {
      // simulate disk full
      TRI_FreeString(TRI_CORE_MEM_ZONE, filename);
      document->_lastError = TRI_set_errno(TRI_ERROR_OUT_OF_MEMORY_MMAP);

      EnsureErrorCode(ENOSPC);

      return nullptr;
    }

    // remove an existing temporary file first
    if (TRI_ExistsFile(filename)) {
      // remove an existing file first
      TRI_UnlinkFile(filename);
    }

    journal = TRI_CreateDatafile(filename, fid, journalSize, true);

    TRI_FreeString(TRI_CORE_MEM_ZONE, filename);
  }

  if (journal == nullptr) {
    if (TRI_errno() == TRI_ERROR_OUT_OF_MEMORY_MMAP) {
      document->_lastError = TRI_set_errno(TRI_ERROR_OUT_OF_MEMORY_MMAP);
    }
    else {
      document->_lastError = TRI_set_errno(TRI_ERROR_ARANGO_NO_JOURNAL);
    }
    
    EnsureErrorCode(document->_lastError);  

    return nullptr;
  }

  // journal is there now
  TRI_ASSERT(journal != nullptr);

  if (isCompactor) {
    LOG_TRACE("created new compactor '%s'", journal->getName(journal));
  }
  else {
    LOG_TRACE("created new journal '%s'", journal->getName(journal));
  }

  // create a collection header, still in the temporary file
  TRI_df_marker_t* position;
  int res = TRI_ReserveElementDatafile(journal, sizeof(TRI_col_header_marker_t), &position, journalSize);
    
  TRI_IF_FAILURE("CreateJournalDocumentCollectionReserve1") {
    res = TRI_ERROR_DEBUG;
  }

  if (res != TRI_ERROR_NO_ERROR) {
    document->_lastError = journal->_lastError;
    LOG_ERROR("cannot create collection header in file '%s': %s", journal->getName(journal), TRI_errno_string(res));

    // close the journal and remove it
    TRI_CloseDatafile(journal);
    TRI_UnlinkFile(journal->getName(journal));
    TRI_FreeDatafile(journal);
    
    EnsureErrorCode(res);  

    return nullptr;
  }

  TRI_col_header_marker_t cm;
  TRI_InitMarkerDatafile((char*) &cm, TRI_COL_MARKER_HEADER, sizeof(TRI_col_header_marker_t));
  cm.base._tick = static_cast<TRI_voc_tick_t>(fid);
  cm._type = (TRI_col_type_t) document->_info._type;
  cm._cid  = document->_info._cid;

  res = TRI_WriteCrcElementDatafile(journal, position, &cm.base, false);

  TRI_IF_FAILURE("CreateJournalDocumentCollectionReserve2") {
    res = TRI_ERROR_DEBUG;
  }

  if (res != TRI_ERROR_NO_ERROR) {
    document->_lastError = journal->_lastError;
    LOG_ERROR("cannot create collection header in file '%s': %s", journal->getName(journal), TRI_last_error());

    // close the journal and remove it
    TRI_CloseDatafile(journal);
    TRI_UnlinkFile(journal->getName(journal));
    TRI_FreeDatafile(journal);
    
    EnsureErrorCode(document->_lastError);  

    return nullptr;
  }

  TRI_ASSERT(fid == journal->_fid);


  // if a physical file, we can rename it from the temporary name to the correct name
  if (! isCompactor) {
    if (journal->isPhysical(journal)) {
      // and use the correct name
      char* number = TRI_StringUInt64(journal->_fid);
      char* jname = TRI_Concatenate3String("journal-", number, ".db");
      char* filename = TRI_Concatenate2File(document->_directory, jname);

      TRI_FreeString(TRI_CORE_MEM_ZONE, number);
      TRI_FreeString(TRI_CORE_MEM_ZONE, jname);

      bool ok = TRI_RenameDatafile(journal, filename);

      if (! ok) {
        LOG_ERROR("failed to rename journal '%s' to '%s': %s", journal->getName(journal), filename, TRI_last_error());

        TRI_CloseDatafile(journal);
        TRI_UnlinkFile(journal->getName(journal));
        TRI_FreeDatafile(journal);
        TRI_FreeString(TRI_CORE_MEM_ZONE, filename);
    
        EnsureErrorCode(document->_lastError);  

        return nullptr;
      }
      else {
        LOG_TRACE("renamed journal from '%s' to '%s'", journal->getName(journal), filename);
      }

      TRI_FreeString(TRI_CORE_MEM_ZONE, filename);
    }

    TRI_PushBackVectorPointer(&document->_journals, journal);
  }

  // now create a datafile entry for the new journal
  TRI_FindDatafileInfoDocumentCollection(document, fid, true);

  return journal;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create an index, based on a JSON description
////////////////////////////////////////////////////////////////////////////////

int TRI_FromJsonIndexDocumentCollection (TRI_document_collection_t* document,
                                         TRI_json_t const* json,
                                         triagens::mvcc::Index*& index) {
  TRI_ASSERT(TRI_IsObjectJson(json));

  index = nullptr;

  // extract the type
  TRI_json_t const* type = TRI_LookupObjectJson(json, "type");

  if (! TRI_IsStringJson(type)) {
    return TRI_ERROR_INTERNAL;
  }

  std::string typeString = std::string(type->_value._string.data, type->_value._string.length - 1);

  // extract the index identifier
  TRI_json_t const* iis = TRI_LookupObjectJson(json, "id");

  TRI_idx_iid_t iid;
  if (TRI_IsNumberJson(iis)) {
    iid = static_cast<TRI_idx_iid_t>(iis->_value._number);
  }
  else if (TRI_IsStringJson(iis)) {
    iid = static_cast<TRI_idx_iid_t>(TRI_UInt64String2(iis->_value._string.data,
                                                       iis->_value._string.length - 1));
  }
  else {
    LOG_ERROR("ignoring index, index identifier could not be located");

    return TRI_ERROR_INTERNAL;
  }

  TRI_UpdateTickServer(iid);

  if (typeString == "cap") {
    return CapConstraintFromJson(document, json, iid, index);
  }

  if (typeString == "geo1" || typeString == "geo2") {
    return GeoIndexFromJson(document, json, iid, index);
  }

  if (typeString == "hash") {
    return HashIndexFromJson(document, json, iid, index);
  }

  if (typeString == "skiplist") {
    return SkiplistIndexFromJson(document, json, iid, index);
  }

  if (typeString == "fulltext") {
    return FulltextIndexFromJson(document, json, iid, index);
  }

  LOG_WARNING("index type '%s' is not supported in this version of ArangoDB and is ignored", 
              typeString.c_str());

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief closes an existing datafile
/// Note that the caller must hold a lock protecting the _datafiles and
/// _journals entry.
////////////////////////////////////////////////////////////////////////////////

bool TRI_CloseDatafileDocumentCollection (TRI_document_collection_t* document,
                                          size_t position,
                                          bool isCompactor) {
  TRI_vector_pointer_t* vector;

  // either use a journal or a compactor
  if (isCompactor) {
    vector = &document->_compactors;
  }
  else {
    vector = &document->_journals;
  }

  // no journal at this position
  if (vector->_length <= position) {
    TRI_set_errno(TRI_ERROR_ARANGO_NO_JOURNAL);
    return false;
  }

  // seal and rename datafile
  TRI_datafile_t* journal = static_cast<TRI_datafile_t*>(vector->_buffer[position]);
  int res = TRI_SealDatafile(journal);

  if (res != TRI_ERROR_NO_ERROR) {
    LOG_ERROR("failed to seal datafile '%s': %s", journal->getName(journal), TRI_last_error());

    if (! isCompactor) {
      TRI_RemoveVectorPointer(vector, position);
      TRI_PushBackVectorPointer(&document->_datafiles, journal);
    }

    return false;
  }

  if (! isCompactor && journal->isPhysical(journal)) {
    // rename the file
    char* number = TRI_StringUInt64(journal->_fid);
    char* dname = TRI_Concatenate3String("datafile-", number, ".db");
    char* filename = TRI_Concatenate2File(document->_directory, dname);

    TRI_FreeString(TRI_CORE_MEM_ZONE, dname);
    TRI_FreeString(TRI_CORE_MEM_ZONE, number);

    bool ok = TRI_RenameDatafile(journal, filename);

    if (! ok) {
      LOG_ERROR("failed to rename datafile '%s' to '%s': %s", journal->getName(journal), filename, TRI_last_error());

      TRI_RemoveVectorPointer(vector, position);
      TRI_PushBackVectorPointer(&document->_datafiles, journal);
      TRI_FreeString(TRI_CORE_MEM_ZONE, filename);

      return false;
    }

    TRI_FreeString(TRI_CORE_MEM_ZONE, filename);

    LOG_TRACE("closed file '%s'", journal->getName(journal));
  }

  if (! isCompactor) {
    TRI_RemoveVectorPointer(vector, position);
    TRI_PushBackVectorPointer(&document->_datafiles, journal);
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief helper struct for filling indexes
////////////////////////////////////////////////////////////////////////////////

class IndexFiller {
  public:
    IndexFiller (TRI_document_collection_t* document,
                 triagens::mvcc::Index* index,
                 std::function<void(int)> callback) 
      : _document(document),
        _index(index),
        _callback(callback) {
    }

    void operator() () {
      TransactionBase trx(true);
      int res = TRI_ERROR_NO_ERROR;

      try {
        _document->fillIndex(_index);
      }
      catch (triagens::arango::Exception const& ex) {
        res = ex.code();
      }
      catch (...) {
        res = TRI_ERROR_INTERNAL;
      }
        
      _callback(res);
    }

  private:

    TRI_document_collection_t* _document;
    triagens::mvcc::Index*     _index;
    std::function<void(int)>   _callback;
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fill the additional (non-primary) indexes
////////////////////////////////////////////////////////////////////////////////

int TRI_FillIndexesDocumentCollection (TRI_vocbase_col_t* collection,
                                       TRI_document_collection_t* document) {
  auto old = document->useSecondaryIndexes();

  // turn filling of secondary indexes off. we're now only interested in getting
  // the indexes' definition. we'll fill them below ourselves.
  document->useSecondaryIndexes(false);

  try {
    TRI_collection_t* collection = reinterpret_cast<TRI_collection_t*>(document);
    TRI_IterateIndexCollection(collection, OpenIndexIterator, collection);
    document->useSecondaryIndexes(old);
  }
  catch (...) {
    document->useSecondaryIndexes(old);
    return TRI_ERROR_INTERNAL;
  }
 
  // TODO TODO TODO: locking!
  auto const indexes = document->indexes();
  size_t const n = indexes.size();
  TRI_ASSERT(n >= 1);
  
  std::atomic<int> result(TRI_ERROR_NO_ERROR);

  // distribute the work to index threads plus this thread
  { 
    triagens::basics::Barrier barrier(n - 1);

    auto indexPool = document->_vocbase->_server->_indexPool;

    auto callback = [&barrier, &result] (int res) -> void {
      // update the error code
      if (res != TRI_ERROR_NO_ERROR) {
        int expected = TRI_ERROR_NO_ERROR;
        result.compare_exchange_strong(expected, res, std::memory_order_acquire);
      }
      
      barrier.join();
    };

    // now actually fill the secondary indexes
    for (size_t i = 1;  i < n;  ++i) {
      auto index = indexes[i];

      // index threads must come first, otherwise this thread will block the loop and
      // prevent distribution to threads
      if (indexPool != nullptr && i != (n - 1)) {
        // move task into thread pool
        IndexFiller indexTask(document, index, callback);

        try {
          static_cast<triagens::basics::ThreadPool*>(indexPool)->enqueue(indexTask);
        }
        catch (...) {
          // set error code
          int expected = TRI_ERROR_NO_ERROR;
          result.compare_exchange_strong(expected, TRI_ERROR_INTERNAL, std::memory_order_acquire);
        
          barrier.join();
        }
      }
      else { 
        // fill index in this thread
        int res = TRI_ERROR_NO_ERROR;
        
        try {
          document->fillIndex(index);
        }
        catch (triagens::arango::Exception const& ex) {
          res = ex.code();
        }
        catch (...) {
          res = TRI_ERROR_INTERNAL;
        }
        
        if (res != TRI_ERROR_NO_ERROR) {
          int expected = TRI_ERROR_NO_ERROR;
          result.compare_exchange_strong(expected, res, std::memory_order_acquire);
        }
        
        barrier.join();
      }
    }

    // barrier waits here until all threads have joined
  }

  return result.load();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief opens an existing collection
////////////////////////////////////////////////////////////////////////////////

TRI_document_collection_t* TRI_OpenDocumentCollection (TRI_vocbase_t* vocbase,
                                                       TRI_vocbase_col_t* col) {
  char const* path = col->_path;

  // first open the document collection
  TRI_document_collection_t* document = nullptr;
  try {
    document = new TRI_document_collection_t();
  }
  catch (std::exception&) {
  }

  if (document == nullptr) {
    return nullptr;
  }

  TRI_ASSERT(document != nullptr);

  TRI_collection_t* collection = TRI_OpenCollection(vocbase, document, path);

  if (collection == nullptr) {
    delete document;
    LOG_ERROR("cannot open document collection from path '%s'", path);

    return nullptr;
  }

  TRI_shaper_t* shaper = TRI_CreateVocShaper(vocbase, document);

  if (shaper == nullptr) {
    LOG_ERROR("cannot create shaper");

    TRI_CloseCollection(collection);
    TRI_FreeCollection(collection);

    return nullptr;
  }

  // create document collection and shaper
  if (false == InitDocumentCollection(document, shaper)) {
    TRI_CloseCollection(collection);
    TRI_FreeCollection(collection);
    LOG_ERROR("cannot initialise document collection");

    return nullptr;
  }

  // check if we can generate the key generator
  KeyGenerator* keyGenerator = KeyGenerator::factory(collection->_info._keyOptions);

  if (keyGenerator == nullptr) {
    TRI_CloseCollection(collection);
    TRI_FreeCollection(collection);
    TRI_set_errno(TRI_ERROR_ARANGO_INVALID_KEY_GENERATOR);

    return nullptr;
  }

  document->_keyGenerator = keyGenerator;

  // create a fake transaction for loading the collection
  TransactionBase trx(true);

  // iterate over all markers of the collection
  int res = IterateMarkersCollection(collection);

  if (res != TRI_ERROR_NO_ERROR) {
    TRI_CloseCollection(collection);
    TRI_FreeCollection(collection);

    LOG_ERROR("cannot iterate data of document collection");
    TRI_set_errno(res);

    return nullptr;
  }

  TRI_ASSERT(document->getShaper() != nullptr);  // ONLY in OPENCOLLECTION, PROTECTED by fake trx here

  TRI_InitVocShaper(document->getShaper());  // ONLY in OPENCOLLECTION, PROTECTED by fake trx here

  if (! triagens::wal::LogfileManager::instance()->isInRecovery()) {
    TRI_FillIndexesDocumentCollection(col, document);
  }

  return document;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief closes an open collection
////////////////////////////////////////////////////////////////////////////////

int TRI_CloseDocumentCollection (TRI_document_collection_t* document,
                                 bool updateStats) {
  auto primaryIndex = document->primaryIndex();

  if (! document->_info._deleted &&
      document->_info._initialCount != static_cast<int64_t>(primaryIndex->size())) {
    // update the document count
    document->_info._initialCount = static_cast<int64_t>(primaryIndex->size());
    
    bool doSync = document->_vocbase->_settings.forceSyncProperties;
    TRI_SaveCollectionInfo(document->_directory, &document->_info, doSync);
  }

  // closes all open compactors, journals, datafiles
  int res = TRI_CloseCollection(document);

  TransactionBase trx(true);  // just to protect the following call
  TRI_FreeVocShaper(document->getShaper());  // ONLY IN CLOSECOLLECTION, PROTECTED by fake trx here
  document->setShaper(nullptr);

  return res;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                           INDEXES
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                     private types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief pid name structure
////////////////////////////////////////////////////////////////////////////////

typedef struct pid_name_s {
  TRI_shape_pid_t _pid;
  char* _name;
}
pid_name_t;

// -----------------------------------------------------------------------------
// --SECTION--                                                 private functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief extracts a field list from a json object
////////////////////////////////////////////////////////////////////////////////

static std::vector<std::string> ExtractAttributes (TRI_json_t const* json) {
  std::vector<std::string> result;

  TRI_json_t const* fields = TRI_LookupObjectJson(json, "fields");

  if (! TRI_IsArrayJson(fields)) {
    return result;
  }

  for (size_t i = 0; i < fields->_value._objects._length;  ++i) {
    auto sub = static_cast<TRI_json_t const*>(TRI_AtVector(&fields->_value._objects, i));

    if (TRI_IsStringJson(sub)) {
      result.emplace_back(std::string(sub->_value._string.data, sub->_value._string.length - 1));
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief finds a path based, unique or non-unique index
////////////////////////////////////////////////////////////////////////////////

static triagens::mvcc::Index* LookupPathIndexDocumentCollection (TRI_document_collection_t* collection,
                                                                 std::vector<TRI_shape_pid_t> const& paths,
                                                                 triagens::mvcc::Index::IndexType type,
                                                                 int sparsity,
                                                                 bool unique,
                                                                 bool allowAnyAttributeOrder) {
  TRI_ASSERT(type == triagens::mvcc::Index::TRI_IDX_TYPE_HASH_INDEX ||
             type == triagens::mvcc::Index::TRI_IDX_TYPE_SKIPLIST_INDEX);

  // TODO: check locking for indexes

  for (auto index : collection->indexes()) {
    if (index->type() != type) {
      // wrong index type
      continue;
    }

    std::vector<TRI_shape_pid_t> indexPaths;

    if (type == triagens::mvcc::Index::TRI_IDX_TYPE_HASH_INDEX) {
      auto found = static_cast<triagens::mvcc::HashIndex*>(index);
      
      int indexSparsity = found->sparse() ? 1 : 0;

      if (found->unique() != unique ||
          (sparsity != -1 && sparsity != indexSparsity)) {
        // different uniqueness
        continue;
      }

      indexPaths = found->paths();
    
    }
    else if (type == triagens::mvcc::Index::TRI_IDX_TYPE_SKIPLIST_INDEX) {
      auto found = static_cast<triagens::mvcc::SkiplistIndex*>(index);
      
      int indexSparsity = found->sparse() ? 1 : 0;
      
      if (found->unique() != unique ||
          (sparsity != -1 && sparsity != indexSparsity)) {
        // different uniqueness
        continue;
      }
      
      indexPaths = found->paths();
    }
    else {
      TRI_ASSERT(false);
    }

    // go through all the attributes and see if they match
    if (paths.size() != indexPaths.size()) {
      // different number of attributes
      continue;
    }


    bool isMatch = true;

    if (allowAnyAttributeOrder) {
      // any permutation of attributes is allowed
      for (auto it : paths) {
        bool found = false;
        for (auto it2 : indexPaths) {
          if (it == it2) {
            found = true;
            break;
          }
        }

        if (! found) {
          isMatch = false;
          break;
        }
      }
    }
    else {
      for (size_t i = 0; i < paths.size(); ++i) {
        if (paths[i] != indexPaths[i]) {
          isMatch = false;
          break;
        }
      }
    }

    if (isMatch) {
      return index;
    }
  }

  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief restores a path based index (template)
////////////////////////////////////////////////////////////////////////////////

static int PathBasedIndexFromJson (TRI_document_collection_t* document,
                                   TRI_json_t const* definition,
                                   TRI_idx_iid_t iid,
                                   triagens::mvcc::Index* (*creator)(TRI_document_collection_t*,
                                                                     std::vector<std::string> const&,
                                                                     TRI_idx_iid_t,
                                                                     bool,
                                                                     bool,
                                                                     bool&),
                                   triagens::mvcc::Index*& dst) {
  dst = nullptr;

  // extract fields
  std::vector<std::string> attributes = ExtractAttributes(definition);

  // extract the list of fields
  if (attributes.size() < 1) {
    LOG_ERROR("ignoring index %llu, need at least one attribute path", 
              (unsigned long long) iid);

    return TRI_set_errno(TRI_ERROR_BAD_PARAMETER);
  }

  // determine if the index is unique or non-unique
  TRI_json_t const* value = TRI_LookupObjectJson(definition, "unique");

  if (! TRI_IsBooleanJson(value)) {
    LOG_ERROR("ignoring index %llu, could not determine if unique or non-unique", 
              (unsigned long long) iid);
    return TRI_set_errno(TRI_ERROR_BAD_PARAMETER);
  }

  bool unique = value->_value._boolean;

  // determine sparsity
  bool sparse = false;

  value = TRI_LookupObjectJson(definition, "sparse"); 

  if (TRI_IsBooleanJson(value)) {
    sparse = value->_value._boolean;
  }
  else {
    // no sparsity information given for index
    // now use pre-2.5 defaults: unique hash indexes were sparse, all other indexes were non-sparse
    bool isHashIndex = false;
    TRI_json_t const* typeJson = TRI_LookupObjectJson(definition, "type");

    if (TRI_IsStringJson(typeJson)) {
      isHashIndex = (strcmp(typeJson->_value._string.data, "hash") == 0);
    }

    if (isHashIndex && unique) {
      sparse = true;
    } 
  }

  // create the index
  bool created;
  triagens::mvcc::Index* index = creator(document, attributes, iid, sparse, unique, created);

  dst = index;

  if (index == nullptr) {
    LOG_ERROR("cannot create index %llu in collection '%s'", (unsigned long long) iid, 
              document->_info._name);
    return TRI_errno();
  }

  return TRI_ERROR_NO_ERROR;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief update statistics for a collection
/// note: the write-lock for the collection must be held to call this
////////////////////////////////////////////////////////////////////////////////

void TRI_UpdateRevisionDocumentCollection (TRI_document_collection_t* document,
                                           TRI_voc_rid_t rid,
                                           bool force) {
  if (rid > 0) {
    SetRevision(document, rid, force);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not a collection is fully collected
////////////////////////////////////////////////////////////////////////////////

bool TRI_IsFullyCollectedDocumentCollection (TRI_document_collection_t* document) {
  TRI_READ_LOCK_DOCUMENTS_INDEXES_PRIMARY_COLLECTION(document);

  int64_t uncollected = document->_uncollectedLogfileEntries;

  TRI_READ_UNLOCK_DOCUMENTS_INDEXES_PRIMARY_COLLECTION(document);

  return (uncollected == 0);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief drops an index, including index file removal and replication
////////////////////////////////////////////////////////////////////////////////

bool TRI_DropIndexDocumentCollection (TRI_document_collection_t* document,
                                      TRI_idx_iid_t iid,
                                      bool writeMarker) {
  // TODO TODO: check if we need to acquire a lock here!
  triagens::mvcc::Index* index = document->lookupIndex(iid);

  if (index == nullptr ||
      index->type() == triagens::mvcc::Index::TRI_IDX_TYPE_PRIMARY_INDEX ||
      index->type() == triagens::mvcc::Index::TRI_IDX_TYPE_EDGE_INDEX) {
    return false;
  }

  if (! document->unlinkIndex(index)) {
    return false;
  }

  return document->dropIndex(index, writeMarker);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief converts attribute names to lists of pids and names
///
/// In case of an error, all allocated memory in pids and names will be
/// freed.
////////////////////////////////////////////////////////////////////////////////

static void PidNamesByAttributeNames (std::vector<std::string> const& attributes,
                                      TRI_shaper_t* shaper,
                                      std::vector<TRI_shape_pid_t>& pids,
                                      std::vector<std::string>& names,
                                      bool sorted,
                                      bool create) {
  pids.reserve(attributes.size());
  names.reserve(attributes.size());

  std::vector<std::pair<TRI_shape_pid_t, std::string>> pidNames;
  pidNames.reserve(attributes.size());

  for (auto it : attributes) {
    TRI_shape_pid_t pid;
    if (create) {
      pid = shaper->findOrCreateAttributePathByName(shaper, it.c_str());
    }
    else {
      pid = shaper->lookupAttributePathByName(shaper, it.c_str());
    }

    if (pid == 0) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_ILLEGAL_NAME);
    }
      
    pidNames.emplace_back(std::make_pair(pid, it));
  }

  if (sorted) {
    // sort according to pid
    std::sort(pidNames.begin(), pidNames.end(), [](std::pair<TRI_shape_pid_t, std::string> const& lhs, 
                                                   std::pair<TRI_shape_pid_t, std::string> const& rhs) {
      return (lhs.first < rhs.first);
    });
  }

  // split again
  for (auto const& it : pidNames) {
    pids.emplace_back(it.first);
    names.emplace_back(it.second);
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                    CAP CONSTRAINT
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                 private functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief adds a cap constraint to a collection
////////////////////////////////////////////////////////////////////////////////

static triagens::mvcc::Index* CreateCapConstraintDocumentCollection (TRI_document_collection_t* document,
                                                                     size_t count,
                                                                     int64_t size,
                                                                     TRI_idx_iid_t iid,
                                                                     bool& created) {
  created = false;

  // create a new index
  if (iid == 0) {
    iid = triagens::mvcc::Index::generateId();
  }

  auto index = new triagens::mvcc::CapConstraint(iid, document, count, size);
  
  try {
    document->addIndex(index);
  }
  catch (...) {
    delete index;
    throw;
  }

  {
    triagens::mvcc::TransactionScope transactionScope(document->_vocbase, triagens::mvcc::TransactionScope::NoCollections());

    auto* transaction = transactionScope.transaction();
    auto* transactionCollection = transaction->collection(document->_info._cid);

    index->apply(transactionCollection, transaction, false);
  }

  created = true;

  return index;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief restores an index
////////////////////////////////////////////////////////////////////////////////

static int CapConstraintFromJson (TRI_document_collection_t* document,
                                  TRI_json_t const* definition,
                                  TRI_idx_iid_t iid,
                                  triagens::mvcc::Index*& dst) {
  dst = nullptr;

  TRI_json_t const* val1 = TRI_LookupObjectJson(definition, "size");
  TRI_json_t const* val2 = TRI_LookupObjectJson(definition, "byteSize");

  if (! TRI_IsNumberJson(val1) && ! TRI_IsNumberJson(val2)) {
    LOG_ERROR("ignoring cap constraint %llu, 'size' and 'byteSize' missing",
              (unsigned long long) iid);

    return TRI_set_errno(TRI_ERROR_BAD_PARAMETER);
  }

  size_t count = 0;
  if (TRI_IsNumberJson(val1) && val1->_value._number > 0.0) {
    count = static_cast<size_t>(val1->_value._number);
  }

  int64_t size = 0;
  if (TRI_IsNumberJson(val2) && val2->_value._number > static_cast<double>(triagens::mvcc::CapConstraint::MinSize)) {
    size = static_cast<int64_t>(val2->_value._number);
  }

  if (count == 0 && size == 0) {
    LOG_ERROR("ignoring cap constraint %llu, 'size' must be at least 1, "
              "or 'byteSize' must be at least %llu",
              (unsigned long long) iid,
              (unsigned long long) triagens::mvcc::CapConstraint::MinSize);

    return TRI_set_errno(TRI_ERROR_BAD_PARAMETER);
  }

  bool created;
  auto index = CreateCapConstraintDocumentCollection(document, count, size, iid, created);

  dst = index;

  return index == nullptr ? TRI_errno() : TRI_ERROR_NO_ERROR;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief looks up a cap constraint
////////////////////////////////////////////////////////////////////////////////

triagens::mvcc::Index* TRI_LookupCapConstraintDocumentCollection (TRI_document_collection_t* document) {
  // check if we already know a cap constraint
#if 0
  // TODO
  if (document->_capConstraint != nullptr) {
    return &document->_capConstraint->base;
  }
#endif

  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief ensures that a cap constraint exists
////////////////////////////////////////////////////////////////////////////////

triagens::mvcc::Index* TRI_EnsureCapConstraintDocumentCollection (TRI_document_collection_t* document,
                                                                  TRI_idx_iid_t iid,
                                                                  size_t count,
                                                                  int64_t size,
                                                                  bool& created) {

  triagens::mvcc::Index* index = CreateCapConstraintDocumentCollection(document, count, size, iid, created);

  if (created) {
    TRI_ASSERT(index != nullptr);
      
    int res = document->saveIndexFile(index, true);

    if (res != TRI_ERROR_NO_ERROR) {
      // TODO: handle failure
      index = nullptr;
    }
  }

  return index;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                         GEO INDEX
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                 private functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief adds a geo index to a collection (location)
////////////////////////////////////////////////////////////////////////////////

static triagens::mvcc::Index* CreateGeoIndexDocumentCollection (TRI_document_collection_t* document,
                                                                std::string const& location,
                                                                bool geoJson,
                                                                TRI_idx_iid_t iid,
                                                                bool& created) {
  created = false;

  triagens::mvcc::Index* index = TRI_LookupGeoIndex1DocumentCollection(document, location, geoJson);

  if (index == nullptr) {
    TRI_shaper_t* shaper = document->getShaper();  // ONLY IN INDEX, PROTECTED by RUNTIME

    TRI_shape_pid_t loc = shaper->findOrCreateAttributePathByName(shaper, location.c_str());

    if (loc == 0) {
      TRI_set_errno(TRI_ERROR_OUT_OF_MEMORY);
      return nullptr;
    }

    std::vector<TRI_shape_pid_t> paths{ loc };
    std::vector<std::string> fields{ location };

    if (iid == 0) {
      iid = triagens::mvcc::Index::generateId();
    }

    index = new triagens::mvcc::GeoIndex(iid, document, fields, paths, geoJson);

    try {
      document->addIndex(index);
    }
    catch (...) {
      delete index;
      throw;
    }
    
    // TODO: fill index, store index

    created = true;
  }

  return index;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief adds a geo index to a collection (latitude, longitude)
////////////////////////////////////////////////////////////////////////////////

static triagens::mvcc::Index* CreateGeoIndexDocumentCollection (TRI_document_collection_t* document,
                                                                std::string const& latitude,
                                                                std::string const& longitude,
                                                                TRI_idx_iid_t iid,
                                                                bool& created) {
  created = false;

  triagens::mvcc::Index* index = TRI_LookupGeoIndex2DocumentCollection(document, latitude, longitude);

  if (index == nullptr) {
    TRI_shaper_t* shaper = document->getShaper();  // ONLY IN INDEX, PROTECTED by RUNTIME

    TRI_shape_pid_t lat = shaper->findOrCreateAttributePathByName(shaper, latitude.c_str());

    if (lat == 0) {
      TRI_set_errno(TRI_ERROR_OUT_OF_MEMORY);
      return nullptr;
    }

    TRI_shape_pid_t lon = shaper->findOrCreateAttributePathByName(shaper, longitude.c_str());

    if (lon == 0) {
      TRI_set_errno(TRI_ERROR_OUT_OF_MEMORY);
      return nullptr;
    }

    std::vector<TRI_shape_pid_t> paths{ lat, lon };
    std::vector<std::string> fields{ latitude, longitude }; 

    if (iid == 0) {
      iid = triagens::mvcc::Index::generateId();
    }

    index = new triagens::mvcc::GeoIndex(iid, document, fields, paths);
  
    try {
      document->addIndex(index);
    }
    catch (...) {
      delete index;
      throw;
    }

    created = true;
  }

  return index;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief restores an index
////////////////////////////////////////////////////////////////////////////////

static int GeoIndexFromJson (TRI_document_collection_t* document,
                             TRI_json_t const* definition,
                             TRI_idx_iid_t iid,
                             triagens::mvcc::Index*& dst) {
  dst = nullptr;

  TRI_json_t const* type = TRI_LookupObjectJson(definition, "type");

  if (! TRI_IsStringJson(type)) {
    return TRI_ERROR_INTERNAL;
  }

  std::string typeString(type->_value._string.data, type->_value._string.length - 1);

  // extract fields
  auto attributes = ExtractAttributes(definition);

  // list style
  if (typeString == "geo1") {
    // extract geo json
    bool geoJson = false;
    auto const value = TRI_LookupObjectJson(definition, "geoJson");

    if (TRI_IsBooleanJson(value)) {
      geoJson = value->_value._boolean;
    }

    // need just one field
    if (attributes.size() == 1) {
      bool created;
      auto index = CreateGeoIndexDocumentCollection(document,
                                                    attributes[0],
                                                    geoJson,
                                                    iid,
                                                    created);

      dst = index;

      return index == nullptr ? TRI_errno() : TRI_ERROR_NO_ERROR;
    }
    else {
      LOG_ERROR("ignoring %s-index %llu, 'fields' must be an array with 1 attribute",
                typeString.c_str(), 
                (unsigned long long) iid);

      return TRI_set_errno(TRI_ERROR_BAD_PARAMETER);
    }
  }

  // attribute style
  else if (typeString == "geo2") {
    if (attributes.size() == 2) {
      bool created;
      auto index = CreateGeoIndexDocumentCollection(document,
                                                    attributes[0],
                                                    attributes[1],
                                                    iid,
                                                    created);

      dst = index;

      return index == nullptr ? TRI_errno() : TRI_ERROR_NO_ERROR;
    }
  }

  return TRI_ERROR_BAD_PARAMETER;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief finds a geo index, list style
////////////////////////////////////////////////////////////////////////////////

triagens::mvcc::Index* TRI_LookupGeoIndex1DocumentCollection (TRI_document_collection_t* document,
                                                              std::string const& location,
                                                              bool geoJson) {
  TRI_shaper_t* shaper = document->getShaper();  // ONLY IN INDEX, PROTECTED by RUNTIME

  TRI_shape_pid_t loc = shaper->lookupAttributePathByName(shaper, location.c_str());

  if (loc == 0) {
    return nullptr;
  }

// TODO TODO TODO
#if 0

  for (auto idx : document->_allIndexes) {
    if (idx->_type == triagens::mvcc::Index::TRI_IDX_TYPE_GEO1_INDEX) {
      TRI_geo_index_t* geo = (TRI_geo_index_t*) idx;

      if (geo->_location != 0 && geo->_location == loc &&
          geo->_geoJson == geoJson) {
        return idx;
      }
    }
  }
#endif

  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief finds a geo index, attribute style
////////////////////////////////////////////////////////////////////////////////

triagens::mvcc::Index* TRI_LookupGeoIndex2DocumentCollection (TRI_document_collection_t* document,
                                                              std::string const& latitude,
                                                              std::string const& longitude) {
  TRI_shaper_t* shaper = document->getShaper();  // ONLY IN INDEX, PROTECTED by RUNTIME

  TRI_shape_pid_t lat = shaper->lookupAttributePathByName(shaper, latitude.c_str());
  TRI_shape_pid_t lon = shaper->lookupAttributePathByName(shaper, longitude.c_str());

  if (lat == 0 || lon == 0) {
    return nullptr;
  }

// TODO TODO TODO
#if 0
  for (auto idx : document->_allIndexes) {
    if (idx->_type == triagens::mvcc::Index::TRI_IDX_TYPE_GEO2_INDEX) {
      TRI_geo_index_t* geo = (TRI_geo_index_t*) idx;

      if (geo->_latitude != 0 &&
          geo->_longitude != 0 &&
          geo->_latitude == lat &&
          geo->_longitude == lon) {
        return idx;
      }
    }
  }
#endif

  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief ensures that a geo index exists, list style
////////////////////////////////////////////////////////////////////////////////

triagens::mvcc::Index* TRI_EnsureGeoIndex1DocumentCollection (TRI_document_collection_t* document,
                                                              TRI_idx_iid_t iid,
                                                              std::string const& location,
                                                              bool geoJson,
                                                              bool& created) {

  triagens::mvcc::Index* index = CreateGeoIndexDocumentCollection(document, location, geoJson, iid, created);

  if (created) {
    TRI_ASSERT(index != nullptr);
      
    int res = document->saveIndexFile(index, true);

    if (res != TRI_ERROR_NO_ERROR) {
      // TODO: handle failure
      index = nullptr;
    }
  }

  return index;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief ensures that a geo index exists, attribute style
////////////////////////////////////////////////////////////////////////////////

triagens::mvcc::Index* TRI_EnsureGeoIndex2DocumentCollection (TRI_document_collection_t* document,
                                                              TRI_idx_iid_t iid,
                                                              std::string const& latitude,
                                                              std::string const& longitude,
                                                              bool& created) {

  triagens::mvcc::Index* index = CreateGeoIndexDocumentCollection(document, latitude, longitude, iid, created);

  if (created) {
    TRI_ASSERT(index != nullptr);
      
    int res = document->saveIndexFile(index, true);

    if (res != TRI_ERROR_NO_ERROR) {
      // TODO: handle failure
      index = nullptr;
    }
  }

  return index;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                        HASH INDEX
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                 private functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief adds a hash index to the collection
////////////////////////////////////////////////////////////////////////////////

static triagens::mvcc::Index* CreateHashIndexDocumentCollection (TRI_document_collection_t* document,
                                                                 std::vector<std::string> const& attributes,
                                                                 TRI_idx_iid_t iid,
                                                                 bool sparse,
                                                                 bool unique,
                                                                 bool& created) {
  std::vector<TRI_shape_pid_t> paths;
  std::vector<std::string> fields;
    
  // determine the sorted shape ids for the attributes
  PidNamesByAttributeNames(attributes,
                           document->getShaper(),  // ONLY IN INDEX, PROTECTED by RUNTIME
                           paths,
                           fields,
                           true,
                           true);

  created = false;

  int sparsity = sparse ? 1 : 0;
  triagens::mvcc::Index* index = LookupPathIndexDocumentCollection(document, paths, triagens::mvcc::Index::TRI_IDX_TYPE_HASH_INDEX, sparsity, unique, false);

  if (index == nullptr) {
    if (iid == 0) {
      iid = triagens::mvcc::Index::generateId();
    }

    index = new triagens::mvcc::HashIndex(iid, document, fields, paths, unique, sparse);
  
    try {
      document->addIndex(index);
    }
    catch (...) {
      delete index;
      throw;
    }
  
    created = true;
    // TODO: fill index, store index
  }
  else {
  }

  return index;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief restores an index
////////////////////////////////////////////////////////////////////////////////

static int HashIndexFromJson (TRI_document_collection_t* document,
                              TRI_json_t const* definition,
                              TRI_idx_iid_t iid,
                              triagens::mvcc::Index*& dst) {
  return PathBasedIndexFromJson(document, definition, iid, CreateHashIndexDocumentCollection, dst);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief finds a hash index (unique or non-unique)
/// the index lock must be held when calling this function
////////////////////////////////////////////////////////////////////////////////

triagens::mvcc::Index* TRI_LookupHashIndexDocumentCollection (TRI_document_collection_t* document,
                                                              std::vector<std::string> const& attributes,
                                                              int sparsity,
                                                              bool unique) {
  std::vector<TRI_shape_pid_t> paths;
  std::vector<std::string> fields;

  try {
    PidNamesByAttributeNames(attributes,
                             document->getShaper(),  // ONLY IN INDEX, PROTECTED by RUNTIME
                             paths,
                             fields,
                             true,
                             false);
  }
  catch (...) {
    // TRI_ERROR_ILLEGAL_NAME here means the index is not there
    return nullptr;
  }

  return LookupPathIndexDocumentCollection(document, paths, triagens::mvcc::Index::TRI_IDX_TYPE_HASH_INDEX, sparsity, unique, true);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief ensures that a hash index exists
////////////////////////////////////////////////////////////////////////////////

triagens::mvcc::Index* TRI_EnsureHashIndexDocumentCollection (TRI_document_collection_t* document,
                                                              TRI_idx_iid_t iid,
                                                              std::vector<std::string> const& attributes,
                                                              bool sparse,
                                                              bool unique,
                                                              bool& created) {

  triagens::mvcc::Index* index = CreateHashIndexDocumentCollection(document, attributes, iid, sparse, unique, created);

  if (created) {
    TRI_ASSERT(index != nullptr);

    int res = document->saveIndexFile(index, true);

    if (res != TRI_ERROR_NO_ERROR) {
      // TODO: handle failure here?
      index = nullptr;
    }
  }

  return index;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                    SKIPLIST INDEX
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                 private functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief adds a skiplist index to the collection
////////////////////////////////////////////////////////////////////////////////

static triagens::mvcc::Index* CreateSkiplistIndexDocumentCollection (TRI_document_collection_t* document,
                                                                     std::vector<std::string> const& attributes,
                                                                     TRI_idx_iid_t iid,
                                                                     bool sparse,
                                                                     bool unique,
                                                                     bool& created) {
  std::vector<TRI_shape_pid_t> paths;
  std::vector<std::string> fields;
  
  PidNamesByAttributeNames(attributes,
                           document->getShaper(),  // ONLY IN INDEX, PROTECTED by RUNTIME
                           paths,
                           fields,
                           false,
                           true);

  created = false;

  int sparsity = sparse ? 1 : 0;
  triagens::mvcc::Index* index = LookupPathIndexDocumentCollection(document, paths, triagens::mvcc::Index::TRI_IDX_TYPE_SKIPLIST_INDEX, sparsity, unique, false);

  if (index == nullptr) {
    if (iid == 0) {
      iid = triagens::mvcc::Index::generateId();
    }

    index = new triagens::mvcc::SkiplistIndex(iid, document, fields, paths, unique, sparse);
  
    try {
      document->addIndex(index);
    }
    catch (...) {
      delete index;
      throw;
    }
  
    created = true;
    // TODO: fill index, store index
  }

  return index;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief restores an index
////////////////////////////////////////////////////////////////////////////////

static int SkiplistIndexFromJson (TRI_document_collection_t* document,
                                  TRI_json_t const* definition,
                                  TRI_idx_iid_t iid,
                                  triagens::mvcc::Index*& dst) {
  return PathBasedIndexFromJson(document, definition, iid, CreateSkiplistIndexDocumentCollection, dst);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief finds a skiplist index (unique or non-unique)
/// the index lock must be held when calling this function
////////////////////////////////////////////////////////////////////////////////

triagens::mvcc::Index* TRI_LookupSkiplistIndexDocumentCollection (TRI_document_collection_t* document,
                                                                  std::vector<std::string> const& attributes,
                                                                  int sparsity,
                                                                  bool unique) {
  std::vector<TRI_shape_pid_t> paths;
  std::vector<std::string> fields;

  try {
    PidNamesByAttributeNames(attributes,
                             document->getShaper(),  // ONLY IN INDEX, PROTECTED by RUNTIME
                             paths,
                             fields,
                             false,
                             false);
  }
  catch (...) {
    // TRI_ERROR_ILLEGAL_NAME here means the index is not there
    return nullptr;
  }

  return LookupPathIndexDocumentCollection(document, paths, triagens::mvcc::Index::TRI_IDX_TYPE_SKIPLIST_INDEX, sparsity, unique, true);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief ensures that a skiplist index exists
////////////////////////////////////////////////////////////////////////////////

triagens::mvcc::Index* TRI_EnsureSkiplistIndexDocumentCollection (TRI_document_collection_t* document,
                                                                  TRI_idx_iid_t iid,
                                                                  std::vector<std::string> const& attributes,
                                                                  bool sparse,
                                                                  bool unique,
                                                                  bool& created) {

  triagens::mvcc::Index* index = CreateSkiplistIndexDocumentCollection(document, attributes, iid, sparse, unique, created);

  if (created) {
    TRI_ASSERT(index != nullptr);
    
    int res = document->saveIndexFile(index, true);

    if (res != TRI_ERROR_NO_ERROR) {
      // TODO: handle failure
      index = nullptr;
    }
  }

  return index;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                    FULLTEXT INDEX
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                 private functions
// -----------------------------------------------------------------------------

triagens::mvcc::Index* LookupFulltextIndexDocumentCollection (TRI_document_collection_t* document,
                                                              std::vector<std::string> const& attributes,
                                                              int minWordLength) {

// TODO: fix fulltext index lookup
#if 0
  for (auto idx : document->_allIndexes) {
    if (idx->_type == triagens::mvcc::Index::TRI_IDX_TYPE_FULLTEXT_INDEX) {
      if (fulltext->_minWordLength != minWordLength) {
        continue;
      }

      if (fulltext->base._fields._length != 1) {
        continue;
      }

      char* fieldName = (char*) fulltext->base._fields._buffer[0];

      if (fieldName != nullptr && TRI_EqualString(fieldName, attributeName)) {
        return idx;
      }
    }
  }
#endif

  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief adds a fulltext index to the collection
////////////////////////////////////////////////////////////////////////////////

static triagens::mvcc::Index* CreateFulltextIndexDocumentCollection (TRI_document_collection_t* document,
                                                                     std::vector<std::string> const& attributes,
                                                                     int minWordLength,
                                                                     TRI_idx_iid_t iid,
                                                                     bool& created) {
  created = false;

  triagens::mvcc::Index* index = LookupFulltextIndexDocumentCollection(document, attributes, minWordLength);

  if (index == nullptr) {
    if (iid == 0) {
      iid = triagens::mvcc::Index::generateId();
    }

    index = new triagens::mvcc::FulltextIndex(iid, document, attributes, minWordLength);
  
    try {
      document->addIndex(index);
    }
    catch (...) {
      delete index;
      throw;
    }

    created = true;
  }

  return index;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief restores an index
////////////////////////////////////////////////////////////////////////////////

static int FulltextIndexFromJson (TRI_document_collection_t* document,
                                  TRI_json_t const* definition,
                                  TRI_idx_iid_t iid,
                                  triagens::mvcc::Index*& dst) {
  dst = nullptr;

  // extract fields
  std::vector<std::string> attributes = ExtractAttributes(definition);

  if (attributes.size() != 1) {
    return TRI_ERROR_BAD_PARAMETER;
  }

  int minWordLengthValue = TRI_FULLTEXT_MIN_WORD_LENGTH_DEFAULT;
  TRI_json_t const* minWordLength = TRI_LookupObjectJson(definition, "minLength");

  if (TRI_IsNumberJson(minWordLength)) {
    minWordLengthValue = static_cast<int>(minWordLength->_value._number);
  }

  // create the index
  triagens::mvcc::Index* index = LookupFulltextIndexDocumentCollection(document, attributes, minWordLengthValue);

  if (index == nullptr) {
    bool created;
    index = CreateFulltextIndexDocumentCollection(document, attributes, minWordLengthValue, iid, created);
  }

  dst = index;

  if (index == nullptr) {
    LOG_ERROR("cannot create fulltext index %llu", (unsigned long long) iid);
    return TRI_ERROR_INTERNAL;
  }

  return TRI_ERROR_NO_ERROR;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief finds a fulltext index (unique or non-unique)
/// the index lock must be held when calling this function
////////////////////////////////////////////////////////////////////////////////

triagens::mvcc::Index* TRI_LookupFulltextIndexDocumentCollection (TRI_document_collection_t* document,
                                                                  std::vector<std::string> const& attributes,
                                                                  int minWordLength) {
  return LookupFulltextIndexDocumentCollection(document, attributes, minWordLength);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief ensures that a fulltext index exists
////////////////////////////////////////////////////////////////////////////////

triagens::mvcc::Index* TRI_EnsureFulltextIndexDocumentCollection (TRI_document_collection_t* document,
                                                                  TRI_idx_iid_t iid,
                                                                  std::vector<std::string> const& attributes,
                                                                  int minWordLength,
                                                                  bool& created) {

  triagens::mvcc::Index* index = CreateFulltextIndexDocumentCollection(document, attributes, minWordLength, iid, created);

  if (created) {
    TRI_ASSERT(index != nullptr);
      
    int res = document->saveIndexFile(index, true);

    if (res != TRI_ERROR_NO_ERROR) {
      // TODO: handle failure
      index = nullptr;
    }
  }

  return index;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief rotate the current journal of the collection
/// use this for testing only
////////////////////////////////////////////////////////////////////////////////

int TRI_RotateJournalDocumentCollection (TRI_document_collection_t* document) {
  int res = TRI_ERROR_ARANGO_NO_JOURNAL;

  TRI_LOCK_JOURNAL_ENTRIES_DOC_COLLECTION(document);

  if (document->_state == TRI_COL_STATE_WRITE) {
    size_t const n = document->_journals._length;

    if (n > 0) {
      TRI_ASSERT(document->_journals._buffer[0] != nullptr);
      TRI_CloseDatafileDocumentCollection(document, 0, false);

      res = TRI_ERROR_NO_ERROR;
    }
  }

  TRI_UNLOCK_JOURNAL_ENTRIES_DOC_COLLECTION(document);

  return res;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
