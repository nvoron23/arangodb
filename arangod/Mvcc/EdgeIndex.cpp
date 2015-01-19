////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC edge index
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

#include "EdgeIndex.h"
#include "Basics/fasthash.h"
#include "Utils/Exception.h"
#include "VocBase/edge-collection.h"
#include "Wal/LogfileManager.h"
#include "Wal/Marker.h"

using namespace triagens::basics;
using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                                 private functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief hashes an edge key
////////////////////////////////////////////////////////////////////////////////

static uint64_t HashElementKey (void const* data) {
  TRI_edge_header_t const* h = static_cast<TRI_edge_header_t const*>(data);
  char const* key = h->_key;

  uint64_t hash = h->_cid;
  hash ^=  fasthash64(key, strlen(key), 0x87654321);

  return fasthash64(&hash, sizeof(hash), 0x56781234);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief hashes an edge (_from case)
////////////////////////////////////////////////////////////////////////////////

static uint64_t HashElementEdgeFrom (void const* data,
                                     bool byKey) {
  uint64_t hash;

  if (! byKey) {
    hash = reinterpret_cast<uint64_t>(data);
  }
  else {
    TRI_doc_mptr_t const* mptr = static_cast<TRI_doc_mptr_t const*>(data);
    TRI_df_marker_t const* marker = static_cast<TRI_df_marker_t const*>(mptr->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

    if (marker->_type == TRI_DOC_MARKER_KEY_EDGE) {
      TRI_doc_edge_key_marker_t const* edge = reinterpret_cast<TRI_doc_edge_key_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
      char const* key = (char const*) edge + edge->_offsetFromKey;

      // LOG_TRACE("HASH FROM: COLLECTION: %llu, KEY: %s", (unsigned long long) edge->_fromCid, key);

      hash = edge->_fromCid;
      hash ^= fasthash64(key, strlen(key), 0x87654321);
    }
    else if (marker->_type == TRI_WAL_MARKER_EDGE) {
      triagens::wal::edge_marker_t const* edge = reinterpret_cast<triagens::wal::edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
      char const* key = (char const*) edge + edge->_offsetFromKey;

      // LOG_TRACE("HASH FROM: COLLECTION: %llu, KEY: %s", (unsigned long long) edge->_fromCid, key);

      hash = edge->_fromCid;
      hash ^= fasthash64(key, strlen(key), 0x87654321);
    }
  }

  return fasthash64(&hash, sizeof(hash), 0x56781234);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief hashes an edge (_to case)
////////////////////////////////////////////////////////////////////////////////

static uint64_t HashElementEdgeTo (void const* data,
                                   bool byKey) {
  uint64_t hash;

  if (! byKey) {
    hash = reinterpret_cast<uint64_t>(data);
  }
  else {
    TRI_doc_mptr_t const* mptr = static_cast<TRI_doc_mptr_t const*>(data);
    TRI_df_marker_t const* marker = static_cast<TRI_df_marker_t const*>(mptr->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

    if (marker->_type == TRI_DOC_MARKER_KEY_EDGE) {
      TRI_doc_edge_key_marker_t const* edge = reinterpret_cast<TRI_doc_edge_key_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
      char const* key = (char const*) edge + edge->_offsetToKey;

      // LOG_TRACE("HASH TO: COLLECTION: %llu, KEY: %s", (unsigned long long) edge->_toCid, key);

      hash = edge->_toCid;
      hash ^= fasthash64(key, strlen(key), 0x87654321);
    }
    else if (marker->_type == TRI_WAL_MARKER_EDGE) {
      triagens::wal::edge_marker_t const* edge = reinterpret_cast<triagens::wal::edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
      char const* key = (char const*) edge + edge->_offsetToKey;

      // LOG_TRACE("HASH TO: COLLECTION: %llu, KEY: %s", (unsigned long long) edge->_toCid, key);

      hash = edge->_toCid;
      hash ^= fasthash64(key, strlen(key), 0x87654321);
    }
  }

  return fasthash64(&hash, sizeof(hash), 0x56781234);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief checks if key and element match (_from case)
////////////////////////////////////////////////////////////////////////////////

static bool IsEqualKeyEdgeFrom (void const* left,
                                void const* right) {
  // left is a key
  // right is an element, that is a master pointer
  TRI_edge_header_t const* l = static_cast<TRI_edge_header_t const*>(left);
  char const* lKey = l->_key;

  TRI_doc_mptr_t const* rMptr = static_cast<TRI_doc_mptr_t const*>(right);
  TRI_df_marker_t const* marker = static_cast<TRI_df_marker_t const*>(rMptr->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

  if (marker->_type == TRI_DOC_MARKER_KEY_EDGE) {
    TRI_doc_edge_key_marker_t const* rEdge = reinterpret_cast<TRI_doc_edge_key_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
    char const* rKey = (char const*) rEdge + rEdge->_offsetFromKey;

    // LOG_TRACE("ISEQUAL FROM: LCOLLECTION: %llu, LKEY: %s, RCOLLECTION: %llu, RKEY: %s", (unsigned long long) l->_cid, lKey, (unsigned long long) rEdge->_fromCid, rKey);
    return (l->_cid == rEdge->_fromCid) && (strcmp(lKey, rKey) == 0);
  }
  else if (marker->_type == TRI_WAL_MARKER_EDGE) {
    triagens::wal::edge_marker_t const* rEdge = reinterpret_cast<triagens::wal::edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
    char const* rKey = (char const*) rEdge + rEdge->_offsetFromKey;

    // LOG_TRACE("ISEQUAL FROM: LCOLLECTION: %llu, LKEY: %s, RCOLLECTION: %llu, RKEY: %s", (unsigned long long) l->_cid, lKey, (unsigned long long) rEdge->_fromCid, rKey);

    return (l->_cid == rEdge->_fromCid) && (strcmp(lKey, rKey) == 0);
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief checks if key and element match (_to case)
////////////////////////////////////////////////////////////////////////////////

static bool IsEqualKeyEdgeTo (void const* left,
                              void const* right) {
  // left is a key
  // right is an element, that is a master pointer
  TRI_edge_header_t const* l = static_cast<TRI_edge_header_t const*>(left);
  char const* lKey = l->_key;

  TRI_doc_mptr_t const* rMptr = static_cast<TRI_doc_mptr_t const*>(right);
  TRI_df_marker_t const* marker = static_cast<TRI_df_marker_t const*>(rMptr->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

  if (marker->_type == TRI_DOC_MARKER_KEY_EDGE) {
    TRI_doc_edge_key_marker_t const* rEdge = reinterpret_cast<TRI_doc_edge_key_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
    char const* rKey = (char const*) rEdge + rEdge->_offsetToKey;

    // LOG_TRACE("ISEQUAL TO: LCOLLECTION: %llu, LKEY: %s, RCOLLECTION: %llu, RKEY: %s", (unsigned long long) l->_cid, lKey, (unsigned long long) rEdge->_toCid, rKey);

    return (l->_cid == rEdge->_toCid) && (strcmp(lKey, rKey) == 0);
  }
  else if (marker->_type == TRI_WAL_MARKER_EDGE) {
    triagens::wal::edge_marker_t const* rEdge = reinterpret_cast<triagens::wal::edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
    char const* rKey = (char const*) rEdge + rEdge->_offsetToKey;

    // LOG_TRACE("ISEQUAL TO: LCOLLECTION: %llu, LKEY: %s, RCOLLECTION: %llu, RKEY: %s", (unsigned long long) l->_cid, lKey, (unsigned long long) rEdge->_toCid, rKey);

    return (l->_cid == rEdge->_toCid) && (strcmp(lKey, rKey) == 0);
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief checks for elements are equal (_from case)
////////////////////////////////////////////////////////////////////////////////

static bool IsEqualElementEdgeFrom (void const* left,
                                    void const* right,
                                    bool byKey) {
  if (! byKey) {
    return left == right;
  }
  else {
    TRI_df_marker_t const* marker;
    char const* lKey;
    TRI_voc_cid_t lCid;
    char const* rKey;
    TRI_voc_cid_t rCid;

    // left element
    TRI_doc_mptr_t const* lMptr = static_cast<TRI_doc_mptr_t const*>(left);
    marker = static_cast<TRI_df_marker_t const*>(lMptr->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

    if (marker->_type == TRI_DOC_MARKER_KEY_EDGE) {
      TRI_doc_edge_key_marker_t const* lEdge = reinterpret_cast<TRI_doc_edge_key_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
      lKey = (char const*) lEdge + lEdge->_offsetFromKey;
      lCid = lEdge->_fromCid;
    }
    else if (marker->_type == TRI_WAL_MARKER_EDGE) {
      triagens::wal::edge_marker_t const* lEdge = reinterpret_cast<triagens::wal::edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
      lKey = (char const*) lEdge + lEdge->_offsetFromKey;
      lCid = lEdge->_fromCid;
    }
    else {
      return false;
    }

    // right element
    TRI_doc_mptr_t const* rMptr = static_cast<TRI_doc_mptr_t const*>(right);
    marker = static_cast<TRI_df_marker_t const*>(rMptr->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

    if (marker->_type == TRI_DOC_MARKER_KEY_EDGE) {
      TRI_doc_edge_key_marker_t const* rEdge = reinterpret_cast<TRI_doc_edge_key_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
      rKey = (char const*) rEdge + rEdge->_offsetFromKey;
      rCid = rEdge->_fromCid;
    }
    else if (marker->_type == TRI_WAL_MARKER_EDGE) {
      triagens::wal::edge_marker_t const* rEdge = reinterpret_cast<triagens::wal::edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
      rKey = (char const*) rEdge + rEdge->_offsetFromKey;
      rCid = rEdge->_fromCid;
    }
    else {
      return false;
    }

    // LOG_TRACE("ISEQUALELEMENT FROM: LCOLLECTION: %llu, LKEY: %s, RCOLLECTION: %llu, RKEY: %s", (unsigned long long) lCid, lKey, (unsigned long long) rCid, rKey);

    return ((lCid == rCid) && (strcmp(lKey, rKey) == 0));
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief checks for elements are equal (_to case)
////////////////////////////////////////////////////////////////////////////////

static bool IsEqualElementEdgeTo (void const* left,
                                  void const* right,
                                  bool byKey) {
  if (! byKey) {
    return left == right;
  }
  else {
    TRI_df_marker_t const* marker;
    char const* lKey;
    TRI_voc_cid_t lCid;
    char const* rKey;
    TRI_voc_cid_t rCid;

    // left element
    TRI_doc_mptr_t const* lMptr = static_cast<TRI_doc_mptr_t const*>(left);
    marker = static_cast<TRI_df_marker_t const*>(lMptr->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

    if (marker->_type == TRI_DOC_MARKER_KEY_EDGE) {
      TRI_doc_edge_key_marker_t const* lEdge = reinterpret_cast<TRI_doc_edge_key_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
      lKey = (char const*) lEdge + lEdge->_offsetToKey;
      lCid = lEdge->_toCid;
    }
    else if (marker->_type == TRI_WAL_MARKER_EDGE) {
      triagens::wal::edge_marker_t const* lEdge = reinterpret_cast<triagens::wal::edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
      lKey = (char const*) lEdge + lEdge->_offsetToKey;
      lCid = lEdge->_toCid;
    }
    else {
      return false;
    }

    // right element
    TRI_doc_mptr_t const* rMptr = static_cast<TRI_doc_mptr_t const*>(right);
    marker = static_cast<TRI_df_marker_t const*>(rMptr->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

    if (marker->_type == TRI_DOC_MARKER_KEY_EDGE) {
      TRI_doc_edge_key_marker_t const* rEdge = reinterpret_cast<TRI_doc_edge_key_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
      rKey = (char const*) rEdge + rEdge->_offsetToKey;
      rCid = rEdge->_toCid;
    }
    else if (marker->_type == TRI_WAL_MARKER_EDGE) {
      triagens::wal::edge_marker_t const* rEdge = reinterpret_cast<triagens::wal::edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
      rKey = (char const*) rEdge + rEdge->_offsetToKey;
      rCid = rEdge->_toCid;
    }
    else {
      return false;
    }

    // LOG_TRACE("ISEQUALELEMENT TO: LCOLLECTION: %llu, LKEY: %s, RCOLLECTION: %llu, RKEY: %s", (unsigned long long) lCid, lKey, (unsigned long long) rCid, rKey);

    return ((lCid == rCid) && (strcmp(lKey, rKey) == 0));
  }
}


// -----------------------------------------------------------------------------
// --SECTION--                                                   class EdgeIndex
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new edge index
////////////////////////////////////////////////////////////////////////////////

EdgeIndex::EdgeIndex (TRI_idx_iid_t id,
                      TRI_document_collection_t* collection) 
  : Index(id, collection, std::vector<std::string>({ TRI_VOC_ATTRIBUTE_FROM, TRI_VOC_ATTRIBUTE_TO })),
    _from(nullptr),
    _to(nullptr) {

    
  _from = new TRI_EdgeIndexHash_t(HashElementKey,
                                  HashElementEdgeFrom,
                                  IsEqualKeyEdgeFrom,
                                  IsEqualElementEdgeFrom);

  _to = new TRI_EdgeIndexHash_t(HashElementKey,
                                HashElementEdgeTo,
                                IsEqualKeyEdgeTo,
                                IsEqualElementEdgeTo);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the edge index
////////////////////////////////////////////////////////////////////////////////

EdgeIndex::~EdgeIndex () {
  if (_to != nullptr) {
    delete _to;
  }

  if (_from != nullptr) {
    delete _from;
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief insert document into index
////////////////////////////////////////////////////////////////////////////////
 
void EdgeIndex::insert (TransactionCollection*, TRI_doc_mptr_t const* doc) {
  
  _from->insert(CONST_CAST(doc), true, false); // OUT
  _to->insert(CONST_CAST(doc), true, false);   // IN
}

////////////////////////////////////////////////////////////////////////////////
/// @brief remove document from index
////////////////////////////////////////////////////////////////////////////////
        
void EdgeIndex::remove (TransactionCollection*, TRI_doc_mptr_t const* doc) {
  _from->remove(doc);  // OUT
  _to->remove(doc);    // IN
}

////////////////////////////////////////////////////////////////////////////////
/// @brief post insert (does nothing)
////////////////////////////////////////////////////////////////////////////////

void EdgeIndex::preCommit (TransactionCollection*) {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief provide a hint for the expected index size
////////////////////////////////////////////////////////////////////////////////

void EdgeIndex::sizeHint (size_t size) {
  // we assume this is called when setting up the index and the index
  // is still empty
  TRI_ASSERT(_from->size() == 0);

  // set an initial size for the index for some new nodes to be created
  // without resizing
  int res = _from->resize(size + 2049);

  if (res != TRI_ERROR_NO_ERROR) {
    THROW_ARANGO_EXCEPTION(res);
  }

  // we assume this is called when setting up the index and the index
  // is still empty
  TRI_ASSERT(_to->size() == 0);

  // set an initial size for the index for some new nodes to be created
  // without resizing
  res = _to->resize(size + 2049);

  if (res != TRI_ERROR_NO_ERROR) {
    THROW_ARANGO_EXCEPTION(res);
  }

}

////////////////////////////////////////////////////////////////////////////////
/// @brief return the amount of memory used by the index
////////////////////////////////////////////////////////////////////////////////
  
size_t EdgeIndex::memory () {
  return _from->memoryUsage() + _to->memoryUsage();
}
      
////////////////////////////////////////////////////////////////////////////////
/// @brief return a JSON representation of the index
////////////////////////////////////////////////////////////////////////////////
      
Json EdgeIndex::toJson (TRI_memory_zone_t* zone) const {
  Json json = Index::toJson(zone);
  Json fields(zone, Json::Array, _fields.size());
  for (auto& field : _fields) {
    fields.add(Json(zone, field));
  }
  json("fields", fields);
  return json;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
