////////////////////////////////////////////////////////////////////////////////
/// @brief mvcc collection open iterator
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
/// @author Copyright 2014, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "OpenIterator.h"
#include "Mvcc/MasterpointerManager.h"
#include "Mvcc/PrimaryIndex.h"
#include "Mvcc/Transaction.h"
#include "Mvcc/TransactionCollection.h"
#include "Mvcc/TransactionId.h"
#include "VocBase/datafile.h"
#include "VocBase/document-collection.h"
#include "VocBase/key-generator.h"
#include "VocBase/voc-shaper.h"

using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                          struct OpenIteratorState
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief creates the open iterator
////////////////////////////////////////////////////////////////////////////////

OpenIteratorState::OpenIteratorState (TRI_document_collection_t* collection)
  : collection(collection),
    datafileId(0),
    revisionId(0),
    documentCount(0),
    documentSize(0),
    dfi(nullptr),
    primaryIndex(nullptr) {

  auto index = collection->lookupIndex(TRI_IDX_TYPE_PRIMARY_INDEX);

  if (index == nullptr) {
    THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_INTERNAL, "no primary index found for collection");
  }

  primaryIndex = static_cast<triagens::mvcc::PrimaryIndex*>(index);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroys the open iterator
////////////////////////////////////////////////////////////////////////////////

OpenIteratorState::~OpenIteratorState () {
  collection->updateDocumentStats(documentCount, documentSize);
  collection->updateRevisionId(revisionId);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------
        
////////////////////////////////////////////////////////////////////////////////
/// @brief looks up a document in the primary index
////////////////////////////////////////////////////////////////////////////////
    
TRI_doc_mptr_t* OpenIteratorState::lookupDocument (char const* key) {
  return primaryIndex->lookup(std::string(key));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief removes a document from the primary index
////////////////////////////////////////////////////////////////////////////////
    
void OpenIteratorState::insertDocument (TRI_df_marker_t const* marker) {
  auto transactionId = triagens::mvcc::TransactionId();

  MasterpointerContainer mptr = collection->masterpointerManager()->create(marker, transactionId);
  mptr->setFrom(transactionId);

  primaryIndex->insert(*mptr);
  
  ++documentCount;
  documentSize += (*mptr)->getDataSize();
  
  mptr.link();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief removes a document from the primary index
////////////////////////////////////////////////////////////////////////////////
    
TRI_doc_mptr_t* OpenIteratorState::removeDocument (char const* key) {
  auto found = primaryIndex->remove(std::string(key));

  if (found != nullptr) {
    --documentCount;
    documentSize -= found->getDataSize();
    collection->masterpointerManager()->unlink(found);
  }

  return found;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief tracks a revision id
////////////////////////////////////////////////////////////////////////////////
    
void OpenIteratorState::trackRevisionId (TRI_voc_rid_t revisionId) {
  if (revisionId > this->revisionId) {
    this->revisionId = revisionId;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief ensures an entry is present for a datafile
////////////////////////////////////////////////////////////////////////////////

void OpenIteratorState::ensureDatafileId (TRI_voc_fid_t fid) {
  if (fid != datafileId) {
    this->dfi = TRI_FindDatafileInfoDocumentCollection(collection, fid, true);

    if (this->dfi == nullptr) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_INTERNAL);
    }

    datafileId = fid;
  }

  TRI_ASSERT_EXPENSIVE(this->dfi != nullptr);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief adds a shape
////////////////////////////////////////////////////////////////////////////////

void OpenIteratorState::addShape (TRI_df_marker_t const* marker) {
  TRI_ASSERT_EXPENSIVE(dfi != nullptr);

  dfi->_numberShapes++;
  dfi->_sizeShapes += (int64_t) TRI_DF_ALIGN_BLOCK(marker->_size);
}
      
////////////////////////////////////////////////////////////////////////////////
/// @brief adds an attribute
////////////////////////////////////////////////////////////////////////////////

void OpenIteratorState::addAttribute (TRI_df_marker_t const* marker) {
  TRI_ASSERT_EXPENSIVE(dfi != nullptr);

  dfi->_numberAttributes++;
  dfi->_sizeAttributes += (int64_t) TRI_DF_ALIGN_BLOCK(marker->_size);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief tracks a document deletion
////////////////////////////////////////////////////////////////////////////////
    
void OpenIteratorState::addDeletion () {
  if (dfi != nullptr) {
    dfi->_numberDeletion++;
  }
}
  
////////////////////////////////////////////////////////////////////////////////
/// @brief tracks a dead document
////////////////////////////////////////////////////////////////////////////////
    
void OpenIteratorState::addDead (TRI_df_marker_t const* marker) {
  int64_t size = static_cast<int64_t>(marker->_size);

  dfi->_numberAlive--;
  dfi->_sizeAlive -= TRI_DF_ALIGN_BLOCK(size);

  dfi->_numberDead++;
  dfi->_sizeDead += TRI_DF_ALIGN_BLOCK(size);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief tracks an alive document
////////////////////////////////////////////////////////////////////////////////
    
void OpenIteratorState::addAlive (TRI_df_marker_t const* marker) {
  dfi->_numberAlive++;
  dfi->_sizeAlive += (int64_t) TRI_DF_ALIGN_BLOCK(marker->_size);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief process a document (or edge) marker when opening a collection
////////////////////////////////////////////////////////////////////////////////

int OpenIterator::handleDocumentMarker (TRI_df_marker_t const* marker,
                                        TRI_datafile_t* datafile,
                                        OpenIteratorState* state) {
  state->ensureDatafileId(datafile->_fid);

  auto* d = reinterpret_cast<TRI_doc_document_key_marker_t const*>(marker);
  state->trackRevisionId(d->_rid);

  auto collection = state->collection;
  auto key = reinterpret_cast<char const*>(d) + d->_offsetKey;
  collection->_keyGenerator->track(key);

  TRI_doc_mptr_t const* found = state->lookupDocument(key);

  if (found == nullptr) {
    // it is a new entry
    state->insertDocument(marker);
    // update the datafile info
    state->addAlive(marker);

    return TRI_ERROR_NO_ERROR;
  }

  // it is an update, but only if found has a smaller revision identifier
  if (found->_rid < d->_rid ||
      (found->_rid == d->_rid && found->_fid <= datafile->_fid)) {
    state->removeDocument(key);
    
    // re-insert with new id
    state->insertDocument(marker);
    // update the datafile info
    state->addAlive(marker);
  }

  // update the datafile info for the datafile the old marker belongs to
  state->ensureDatafileId(found->_fid);
  state->addDead(static_cast<TRI_df_marker_t const*>(found->getDataPtr()));

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief process a deletion marker when opening a collection
////////////////////////////////////////////////////////////////////////////////

int OpenIterator::handleDeletionMarker (TRI_df_marker_t const* marker,
                                        TRI_datafile_t* datafile,
                                        OpenIteratorState* state) {
  state->ensureDatafileId(datafile->_fid);

  auto* d = reinterpret_cast<TRI_doc_deletion_key_marker_t const*>(marker);
  state->trackRevisionId(d->_rid);

  auto collection = state->collection;
  auto key = reinterpret_cast<char const*>(d) + d->_offsetKey;
  collection->_keyGenerator->track(key);

  TRI_doc_mptr_t* found = state->removeDocument(key);

  if (found != nullptr) {
    // it is a real delete
    state->addDead(static_cast<TRI_df_marker_t const*>(found->getDataPtr()));
  }
    
  state->addDeletion();

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief process a shape marker when opening a collection
////////////////////////////////////////////////////////////////////////////////

int OpenIterator::handleShapeMarker (TRI_df_marker_t const* marker,
                                     TRI_datafile_t* datafile,
                                     OpenIteratorState* state) {
  int res = TRI_InsertShapeVocShaper(state->collection->getShaper(), marker, true); 

  if (res == TRI_ERROR_NO_ERROR) {
    state->ensureDatafileId(datafile->_fid);
    state->addShape(marker);
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief process an attribute marker when opening a collection
////////////////////////////////////////////////////////////////////////////////

int OpenIterator::handleAttributeMarker (TRI_df_marker_t const* marker,
                                         TRI_datafile_t* datafile,
                                         OpenIteratorState* state) {
  int res = TRI_InsertAttributeVocShaper(state->collection->getShaper(), marker, true);  

  if (res == TRI_ERROR_NO_ERROR) {
    state->ensureDatafileId(datafile->_fid);
    state->addAttribute(marker);
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief mvcc iterator used when opening a collection
////////////////////////////////////////////////////////////////////////////////

bool OpenIterator::execute (TRI_df_marker_t const* marker,
                            void* data,
                            TRI_datafile_t* datafile) {
  auto state = static_cast<OpenIteratorState*>(data);

  int res = TRI_ERROR_NO_ERROR;

  if (marker->_type == TRI_DOC_MARKER_KEY_EDGE ||
      marker->_type == TRI_DOC_MARKER_KEY_DOCUMENT) {
    res = handleDocumentMarker(marker, datafile, state);
  }
  else if (marker->_type == TRI_DOC_MARKER_KEY_DELETION) {
    res = handleDeletionMarker(marker, datafile, state);
  }
  else if (marker->_type == TRI_DF_MARKER_SHAPE) {
    res = handleShapeMarker(marker, datafile, state);
  }
  else if (marker->_type == TRI_DF_MARKER_ATTRIBUTE) {
    res = handleAttributeMarker(marker, datafile, state);
  }

  TRI_voc_tick_t tick = marker->_tick;

  if (datafile->_tickMin == 0) {
    datafile->_tickMin = tick;
  }

  if (tick > datafile->_tickMax) {
    datafile->_tickMax = tick;
  }

  // set tick values for data markers (document/edge), too
  if (marker->_type == TRI_DOC_MARKER_KEY_DOCUMENT ||
      marker->_type == TRI_DOC_MARKER_KEY_EDGE) {

    if (datafile->_dataMin == 0) {
      datafile->_dataMin = tick;
    }

    if (tick > datafile->_dataMax) {
      datafile->_dataMax = tick;
    }
  }

  auto collection = state->collection;

  if (tick > collection->_tickMax) {
    if (marker->_type != TRI_DF_MARKER_HEADER &&
        marker->_type != TRI_DF_MARKER_FOOTER && 
        marker->_type != TRI_COL_MARKER_HEADER) { 
      collection->_tickMax = tick;
    }
  }

  return (res == TRI_ERROR_NO_ERROR);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
