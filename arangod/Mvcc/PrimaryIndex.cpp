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
/// @author Max Neunhoeffer
/// @author Copyright 2015, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "PrimaryIndex.h"
#include "Basics/fasthash.h"
#include "Utils/Exception.h"
#include "VocBase/document-collection.h"

using namespace triagens::basics;
using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                                class PrimaryIndex
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                     hash and comparison functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief hash function only looking at the key
////////////////////////////////////////////////////////////////////////////////

static inline uint64_t hashKey (char const* key) {
  size_t len = strlen(key);
  return fasthash64(key, len, 0x13579864);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief hash function looking at _key and _rev or only at _key
////////////////////////////////////////////////////////////////////////////////

static uint64_t hashElement (TRI_doc_mptr_t const* elm, bool byKey) {
  TRI_df_marker_t const* marker = static_cast<TRI_df_marker_t const*>(elm->getDataPtr());
  TRI_ASSERT(marker != nullptr);
  char const* charMarker = static_cast<char const*>(elm->getDataPtr());

  uint64_t hash;

  if (marker->_type == TRI_DOC_MARKER_KEY_DOCUMENT ||
      marker->_type == TRI_DOC_MARKER_KEY_EDGE) {
    auto offset = (reinterpret_cast<TRI_doc_document_key_marker_t const*>(marker))->_offsetKey;
    hash = hashKey(charMarker + offset);
  }
  else if (marker->_type == TRI_WAL_MARKER_DOCUMENT ||
           marker->_type == TRI_WAL_MARKER_EDGE) {
    auto offset = (reinterpret_cast<triagens::wal::document_marker_t const*>(marker))->_offsetKey;
    hash = hashKey(charMarker + offset);
  }
  else {
    TRI_ASSERT(false);
  }
  if (byKey) {
    return hash;
  }
  return fasthash64(&elm->_rid, sizeof(elm->_rid), hash);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief comparison function key/element
////////////////////////////////////////////////////////////////////////////////

static bool compareKeyElement(char const* key,
                              TRI_doc_mptr_t const* elm) {
  TRI_df_marker_t const* marker = static_cast<TRI_df_marker_t const*>(elm->getDataPtr());
  TRI_ASSERT(marker != nullptr);
  char const* charMarker = static_cast<char const*>(elm->getDataPtr());
  if (marker->_type == TRI_DOC_MARKER_KEY_DOCUMENT ||
      marker->_type == TRI_DOC_MARKER_KEY_EDGE) {
    auto offset = (reinterpret_cast<TRI_doc_document_key_marker_t const*>(marker))->_offsetKey;
    return strcmp(key, charMarker + offset) == 0;
  }
  else if (marker->_type == TRI_WAL_MARKER_DOCUMENT ||
           marker->_type == TRI_WAL_MARKER_EDGE) {
    auto offset = (reinterpret_cast<triagens::wal::document_marker_t const*>(marker))->_offsetKey;
    return strcmp(key, charMarker + offset) == 0;
  }
  TRI_ASSERT(false);
  return false;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief comparison function element/element
////////////////////////////////////////////////////////////////////////////////

static bool compareElementElement(TRI_doc_mptr_t const* left,
                                  TRI_doc_mptr_t const* right,
                                  bool byKey) {
  TRI_df_marker_t const* lmarker = static_cast<TRI_df_marker_t const*>(left->getDataPtr());
  TRI_ASSERT(lmarker != nullptr);
  char const* lCharMarker = static_cast<char const*>(left->getDataPtr());

  TRI_df_marker_t const* rmarker = static_cast<TRI_df_marker_t const*>(right->getDataPtr());
  TRI_ASSERT(rmarker != nullptr);
  char const* rCharMarker = static_cast<char const*>(right->getDataPtr());

  char const* leftKey;
  char const* rightKey;

  if (lmarker->_type == TRI_DOC_MARKER_KEY_DOCUMENT ||
      lmarker->_type == TRI_DOC_MARKER_KEY_EDGE) {
    auto offset = (reinterpret_cast<TRI_doc_document_key_marker_t const*>(lmarker))->_offsetKey;
    leftKey = lCharMarker + offset;
  }
  else if (lmarker->_type == TRI_WAL_MARKER_DOCUMENT ||
           lmarker->_type == TRI_WAL_MARKER_EDGE) {
    auto offset = (reinterpret_cast<triagens::wal::document_marker_t const*>(lmarker))->_offsetKey;
    leftKey = lCharMarker + offset;
  }
  else {
    TRI_ASSERT(false);
  }

  if (rmarker->_type == TRI_DOC_MARKER_KEY_DOCUMENT ||
      rmarker->_type == TRI_DOC_MARKER_KEY_EDGE) {
    auto offset = (reinterpret_cast<TRI_doc_document_key_marker_t const*>(rmarker))->_offsetKey;
    rightKey = rCharMarker + offset;
  }
  else if (rmarker->_type == TRI_WAL_MARKER_DOCUMENT ||
           rmarker->_type == TRI_WAL_MARKER_EDGE) {
    auto offset = (reinterpret_cast<triagens::wal::document_marker_t const*>(rmarker))->_offsetKey;
    rightKey = rCharMarker + offset;
  }
  else {
    TRI_ASSERT(false);
  }

  if (strcmp(leftKey, rightKey) != 0) {
    return false;
  }
  if (byKey) {
    return true;
  }
  return left->_rid == right->_rid;
}


// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new primary index
////////////////////////////////////////////////////////////////////////////////

PrimaryIndex::PrimaryIndex (TRI_idx_iid_t id,
                            TRI_document_collection_t* collection)
  : Index(id, collection, 
          std::vector<std::string>( { TRI_VOC_ATTRIBUTE_KEY } )) {
  
  _theHash = new PrimaryIndexHash_t(hashKey, hashElement,
                                    compareKeyElement,
                                    compareElementElement);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the hash index
////////////////////////////////////////////////////////////////////////////////

PrimaryIndex::~PrimaryIndex () {
  delete _theHash;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief insert a document into the index
////////////////////////////////////////////////////////////////////////////////
        
void PrimaryIndex::insert (TransactionCollection*,
                           TRI_doc_mptr_t const* doc) {
  TRI_doc_mptr_t const* old;
  
  old = _theHash->insert(doc, false, true);
  if (old != nullptr) {
    // This is serious: a document with this exact key and revision already
    // exists!
    THROW_ARANGO_EXCEPTION(TRI_ERROR_KEYVALUE_KEY_EXISTS);
  }
  // Note that this is not the place to check whether a document with the
  // same key already exists! It might have been deleted since but still
  // resides in the primary index for older transactions to see!
  // Therefore no more check is needed here.
}

////////////////////////////////////////////////////////////////////////////////
/// @brief remove a document from the index
////////////////////////////////////////////////////////////////////////////////

void PrimaryIndex::remove (TransactionCollection*,
                           TRI_doc_mptr_t const*) {
  // This is a no op, since we need to keep old revisions around in the index.
  // See forget for the actual removal of the document from the index.
}        

////////////////////////////////////////////////////////////////////////////////
/// @brief forget a document in the index
////////////////////////////////////////////////////////////////////////////////

void PrimaryIndex::forget (TransactionCollection*,
                           TRI_doc_mptr_t const* doc) {
  TRI_doc_mptr_t const* old;

  old = _theHash->remove(doc);
  if (old == nullptr) {
    // This is serious and unexpected, this revision was not found!
    THROW_ARANGO_EXCEPTION(TRI_ERROR_KEYVALUE_KEY_NOT_FOUND);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief post insert (does nothing)
////////////////////////////////////////////////////////////////////////////////
        
void PrimaryIndex::preCommit (TransactionCollection*) {
  // This is a no op, since the primary index is not used for pre commit
  // cleanup.
}

////////////////////////////////////////////////////////////////////////////////
/// @brief provides a hint for the expected index size
////////////////////////////////////////////////////////////////////////////////

void PrimaryIndex::sizeHint (size_t size) {
  int res = _theHash->resize(3*size+1);   // Take into account old revisions
  if (res == TRI_ERROR_OUT_OF_MEMORY) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_OUT_OF_MEMORY);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the memory used by the index
////////////////////////////////////////////////////////////////////////////////
  
size_t PrimaryIndex::memory () {
  return _theHash->memoryUsage();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief return a JSON representation of the index
////////////////////////////////////////////////////////////////////////////////

Json PrimaryIndex::toJson (TRI_memory_zone_t* zone) const {
  Json json = Index::toJson(zone);
  Json fields(zone, Json::Array, _fields.size());
  json("fields", Json(zone, Json::Array, 1)
                     (Json(zone, "_key")));
  return json;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
