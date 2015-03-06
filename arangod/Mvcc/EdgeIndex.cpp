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
#include "Basics/ReadLocker.h"
#include "Basics/WriteLocker.h"
#include "Mvcc/Transaction.h"
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
/// @brief check whether the _from and _to end of an edge are identical
////////////////////////////////////////////////////////////////////////////////

static bool IsReflexive (TRI_doc_mptr_t const* mptr) {
  TRI_df_marker_t const* marker = static_cast<TRI_df_marker_t const*>(mptr->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

  if (marker->_type == TRI_DOC_MARKER_KEY_EDGE) {
    auto* edge = static_cast<TRI_doc_edge_key_marker_t const*>(mptr->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

    if (edge->_toCid == edge->_fromCid) {
      char const* fromKey = reinterpret_cast<char const*>(edge) + edge->_offsetFromKey;
      char const* toKey =   reinterpret_cast<char const*>(edge) + edge->_offsetToKey;

      return strcmp(fromKey, toKey) == 0;
    }
  }
  else if (marker->_type == TRI_WAL_MARKER_EDGE) {
    auto* edge = static_cast<triagens::wal::edge_marker_t const*>(mptr->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

    if (edge->_toCid == edge->_fromCid) {
      char const* fromKey = reinterpret_cast<char const*>(edge) + edge->_offsetFromKey;
      char const* toKey =   reinterpret_cast<char const*>(edge) + edge->_offsetToKey;

      return strcmp(fromKey, toKey) == 0;
    }
  }
  else if (marker->_type == TRI_WAL_MARKER_MVCC_EDGE) {
    auto* edge = static_cast<triagens::wal::mvcc_edge_marker_t const*>(mptr->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

    if (edge->_toCid == edge->_fromCid) {
      char const* fromKey = reinterpret_cast<char const*>(edge) + edge->_offsetFromKey;
      char const* toKey =   reinterpret_cast<char const*>(edge) + edge->_offsetToKey;

      return strcmp(fromKey, toKey) == 0;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief hash function for key and collection id
////////////////////////////////////////////////////////////////////////////////

static inline uint32_t HashKeyCid (char const* key, 
                                   TRI_voc_cid_t cid) {
  uint32_t hash = TRI_64to32(cid);
  hash ^= fasthash32(key, strlen(key), 0x87654321);
  return fasthash32(&hash, sizeof(hash), 0x56781234);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief hashes an edge key
////////////////////////////////////////////////////////////////////////////////

static uint32_t HashElementKey (TRI_edge_header_t const* header) {
  return HashKeyCid(header->_key, header->_cid);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief hashes an edge (_from case)
////////////////////////////////////////////////////////////////////////////////

static uint64_t HashElementEdgeFrom (void const* data,
                                     bool byKey) {
  if (byKey) {
    TRI_doc_mptr_t const* mptr = static_cast<TRI_doc_mptr_t const*>(data);
    TRI_df_marker_t const* marker = static_cast<TRI_df_marker_t const*>(mptr->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME
    char const* key = nullptr;
    TRI_voc_cid_t cid = 0;

    if (marker->_type == TRI_DOC_MARKER_KEY_EDGE) {
      auto* edge = reinterpret_cast<TRI_doc_edge_key_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
      key = (char const*) edge + edge->_offsetFromKey;
      cid = edge->_fromCid;
    }
    else if (marker->_type == TRI_WAL_MARKER_EDGE) {
      auto* edge = reinterpret_cast<triagens::wal::edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
      key = (char const*) edge + edge->_offsetFromKey;
      cid = edge->_fromCid;
    }
    else if (marker->_type == TRI_WAL_MARKER_MVCC_EDGE) {
      auto* edge = reinterpret_cast<triagens::wal::mvcc_edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
      key = (char const*) edge + edge->_offsetFromKey;
      cid = edge->_fromCid;
    }
      
    return HashKeyCid(key, cid);
  }
    
  uint32_t hash = TRI_64to32(reinterpret_cast<uintptr_t>(data));
  return fasthash32(&hash, sizeof(hash), 0x56781234);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief hashes an edge (_to case)
////////////////////////////////////////////////////////////////////////////////

static uint64_t HashElementEdgeTo (void const* data,
                                   bool byKey) {
  if (byKey) {
    TRI_doc_mptr_t const* mptr = static_cast<TRI_doc_mptr_t const*>(data);
    TRI_df_marker_t const* marker = static_cast<TRI_df_marker_t const*>(mptr->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME
    char const* key = nullptr;
    TRI_voc_cid_t cid = 0;

    if (marker->_type == TRI_DOC_MARKER_KEY_EDGE) {
      auto* edge = reinterpret_cast<TRI_doc_edge_key_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
      key = (char const*) edge + edge->_offsetToKey;
      cid = edge->_toCid;
    }
    else if (marker->_type == TRI_WAL_MARKER_EDGE) {
      auto* edge = reinterpret_cast<triagens::wal::edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
      key = (char const*) edge + edge->_offsetToKey;
      cid = edge->_toCid;
    }
    else if (marker->_type == TRI_WAL_MARKER_MVCC_EDGE) {
      auto* edge = reinterpret_cast<triagens::wal::mvcc_edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
      key = (char const*) edge + edge->_offsetToKey;
      cid = edge->_toCid;
    }
    
    return HashKeyCid(key, cid);
  }
    
  uint32_t hash = TRI_64to32(reinterpret_cast<uintptr_t>(data));
  return fasthash32(&hash, sizeof(hash), 0x56781234);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief checks if key and element match (_from case)
////////////////////////////////////////////////////////////////////////////////

static bool IsEqualKeyEdgeFrom (TRI_edge_header_t const* left,
                                TRI_doc_mptr_t const* right) {
  // left is a key
  // right is an element, that is a master pointer
  char const* lKey = left->_key;

  TRI_df_marker_t const* marker = static_cast<TRI_df_marker_t const*>(right->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

  if (marker->_type == TRI_DOC_MARKER_KEY_EDGE) {
    auto* rEdge = reinterpret_cast<TRI_doc_edge_key_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
    char const* rKey = (char const*) rEdge + rEdge->_offsetFromKey;
    return (left->_cid == rEdge->_fromCid) && (strcmp(lKey, rKey) == 0);
  }
  else if (marker->_type == TRI_WAL_MARKER_EDGE) {
    auto* rEdge = reinterpret_cast<triagens::wal::edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
    char const* rKey = (char const*) rEdge + rEdge->_offsetFromKey;

    return (left->_cid == rEdge->_fromCid) && (strcmp(lKey, rKey) == 0);
  }
  else if (marker->_type == TRI_WAL_MARKER_MVCC_EDGE) {
    auto* rEdge = reinterpret_cast<triagens::wal::mvcc_edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
    char const* rKey = (char const*) rEdge + rEdge->_offsetFromKey;

    return (left->_cid == rEdge->_fromCid) && (strcmp(lKey, rKey) == 0);
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief checks if key and element match (_to case)
////////////////////////////////////////////////////////////////////////////////

static bool IsEqualKeyEdgeTo (TRI_edge_header_t const* left,
                              TRI_doc_mptr_t const* right) {
  // left is a key
  // right is an element, that is a master pointer
  char const* lKey = left->_key;

  TRI_df_marker_t const* marker = static_cast<TRI_df_marker_t const*>(right->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

  if (marker->_type == TRI_DOC_MARKER_KEY_EDGE) {
    auto* rEdge = reinterpret_cast<TRI_doc_edge_key_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
    char const* rKey = (char const*) rEdge + rEdge->_offsetToKey;

    return (left->_cid == rEdge->_toCid) && (strcmp(lKey, rKey) == 0);
  }
  else if (marker->_type == TRI_WAL_MARKER_EDGE) {
    auto* rEdge = reinterpret_cast<triagens::wal::edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
    char const* rKey = (char const*) rEdge + rEdge->_offsetToKey;

    return (left->_cid == rEdge->_toCid) && (strcmp(lKey, rKey) == 0);
  }
  else if (marker->_type == TRI_WAL_MARKER_MVCC_EDGE) {
    auto* rEdge = reinterpret_cast<triagens::wal::mvcc_edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
    char const* rKey = (char const*) rEdge + rEdge->_offsetToKey;

    return (left->_cid == rEdge->_toCid) && (strcmp(lKey, rKey) == 0);
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief checks for elements are equal (_from case)
////////////////////////////////////////////////////////////////////////////////

static bool IsEqualElementEdgeFrom (TRI_doc_mptr_t const* left,
                                    TRI_doc_mptr_t const* right,
                                    bool byKey) {
  TRI_df_marker_t const* marker;
  char const* lKey;
  TRI_voc_cid_t lCid;
  TRI_voc_rid_t lRid;
  char const* rKey;
  TRI_voc_cid_t rCid;
  TRI_voc_rid_t rRid;

  // left element
  marker = static_cast<TRI_df_marker_t const*>(left->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

  if (marker->_type == TRI_DOC_MARKER_KEY_EDGE) {
    auto* lEdge = reinterpret_cast<TRI_doc_edge_key_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
    lKey = (char const*) lEdge + lEdge->_offsetFromKey;
    lCid = lEdge->_fromCid;
    lRid = lEdge->base._rid;
  }
  else if (marker->_type == TRI_WAL_MARKER_EDGE) {
    auto* lEdge = reinterpret_cast<triagens::wal::edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
    lKey = (char const*) lEdge + lEdge->_offsetFromKey;
    lCid = lEdge->_fromCid;
    lRid = lEdge->_revisionId;
  }
  else if (marker->_type == TRI_WAL_MARKER_MVCC_EDGE) {
    auto* lEdge = reinterpret_cast<triagens::wal::mvcc_edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
    lKey = (char const*) lEdge + lEdge->_offsetFromKey;
    lCid = lEdge->_fromCid;
    lRid = lEdge->_revisionId;
  }
  else {
    return false;
  }

  // right element
  marker = static_cast<TRI_df_marker_t const*>(right->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

  if (marker->_type == TRI_DOC_MARKER_KEY_EDGE) {
    auto* rEdge = reinterpret_cast<TRI_doc_edge_key_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
    rKey = (char const*) rEdge + rEdge->_offsetFromKey;
    rCid = rEdge->_fromCid;
    rRid = rEdge->base._rid;
  }
  else if (marker->_type == TRI_WAL_MARKER_EDGE) {
    auto* rEdge = reinterpret_cast<triagens::wal::edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
    rKey = (char const*) rEdge + rEdge->_offsetFromKey;
    rCid = rEdge->_fromCid;
    rRid = rEdge->_revisionId;
  }
  else if (marker->_type == TRI_WAL_MARKER_MVCC_EDGE) {
    auto* rEdge = reinterpret_cast<triagens::wal::mvcc_edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
    rKey = (char const*) rEdge + rEdge->_offsetFromKey;
    rCid = rEdge->_fromCid;
    rRid = rEdge->_revisionId;
  }
  else {
    return false;
  }

  if (byKey) { 
    return ((lCid == rCid) && (strcmp(lKey, rKey) == 0));
  }

  return ((lRid == rRid) && (lCid == rCid) && (strcmp(lKey, rKey) == 0));
}

////////////////////////////////////////////////////////////////////////////////
/// @brief checks for elements are equal (_to case)
////////////////////////////////////////////////////////////////////////////////

static bool IsEqualElementEdgeTo (TRI_doc_mptr_t const* left,
                                  TRI_doc_mptr_t const* right,
                                  bool byKey) {
  TRI_df_marker_t const* marker;
  char const* lKey;
  TRI_voc_cid_t lCid;
  TRI_voc_cid_t lRid;
  char const* rKey;
  TRI_voc_cid_t rCid;
  TRI_voc_cid_t rRid;

  // left element
  marker = static_cast<TRI_df_marker_t const*>(left->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

  if (marker->_type == TRI_DOC_MARKER_KEY_EDGE) {
    auto* lEdge = reinterpret_cast<TRI_doc_edge_key_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
    lKey = (char const*) lEdge + lEdge->_offsetToKey;
    lCid = lEdge->_toCid;
    lRid = lEdge->base._rid;
  }
  else if (marker->_type == TRI_WAL_MARKER_EDGE) {
    auto* lEdge = reinterpret_cast<triagens::wal::edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
    lKey = (char const*) lEdge + lEdge->_offsetToKey;
    lCid = lEdge->_toCid;
    lRid = lEdge->_revisionId;
  }
  else if (marker->_type == TRI_WAL_MARKER_MVCC_EDGE) {
    auto* lEdge = reinterpret_cast<triagens::wal::mvcc_edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
    lKey = (char const*) lEdge + lEdge->_offsetToKey;
    lCid = lEdge->_toCid;
    lRid = lEdge->_revisionId;
  }
  else {
    return false;
  }

  // right element
  marker = static_cast<TRI_df_marker_t const*>(right->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

  if (marker->_type == TRI_DOC_MARKER_KEY_EDGE) {
    auto* rEdge = reinterpret_cast<TRI_doc_edge_key_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
    rKey = (char const*) rEdge + rEdge->_offsetToKey;
    rCid = rEdge->_toCid;
    rRid = rEdge->base._rid;
  }
  else if (marker->_type == TRI_WAL_MARKER_EDGE) {
    auto* rEdge = reinterpret_cast<triagens::wal::edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
    rKey = (char const*) rEdge + rEdge->_offsetToKey;
    rCid = rEdge->_toCid;
    rRid = rEdge->_revisionId;
  }
  else if (marker->_type == TRI_WAL_MARKER_MVCC_EDGE) {
    auto* rEdge = reinterpret_cast<triagens::wal::mvcc_edge_marker_t const*>(marker);  // ONLY IN INDEX, PROTECTED by RUNTIME
    rKey = (char const*) rEdge + rEdge->_offsetToKey;
    rCid = rEdge->_toCid;
    rRid = rEdge->_revisionId;
  }
  else {
    return false;
  }

  if (byKey) {
    return ((lCid == rCid) && (strcmp(lKey, rKey) == 0));
  }

  return ((lRid == rRid) && (lCid == rCid) && (strcmp(lKey, rKey) == 0));
}

// -----------------------------------------------------------------------------
// --SECTION--                                                   class EdgeIndex
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new edge index (not fully functional - use only to 
/// retrieve the JSON representation of the index)
////////////////////////////////////////////////////////////////////////////////

EdgeIndex::EdgeIndex (TRI_idx_iid_t id)
  : Index(id, std::vector<std::string>({ TRI_VOC_ATTRIBUTE_FROM, TRI_VOC_ATTRIBUTE_TO })),
    _collection(nullptr),
    _lock(),
    _from(nullptr),
    _to(nullptr) {

}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new edge index
////////////////////////////////////////////////////////////////////////////////

EdgeIndex::EdgeIndex (TRI_idx_iid_t id,
                      TRI_document_collection_t* collection) 
  : Index(id, std::vector<std::string>({ TRI_VOC_ATTRIBUTE_FROM, TRI_VOC_ATTRIBUTE_TO })),
    _collection(collection),
    _lock(),
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
  delete _to;
  delete _from;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief insert document into index (called when opening the collection)
////////////////////////////////////////////////////////////////////////////////
 
void EdgeIndex::insert (TRI_doc_mptr_t* doc) {
  _from->insert(doc, false, false);

  try {
    _to->insert(doc, false, false);
  }
  catch (...) {
    _from->remove(doc);
    // insert into to failed, now rollback insert into from
    throw;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief insert document into index
////////////////////////////////////////////////////////////////////////////////
 
void EdgeIndex::insert (TransactionCollection*, 
                        Transaction*, 
                        TRI_doc_mptr_t* doc) {
  WRITE_LOCKER(_lock);

  _from->insert(doc, false, false);

  try {
    _to->insert(doc, false, false); 
  }
  catch (...) {
    _from->remove(doc);
    // insert into to failed, now rollback insert into from
    throw;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief remove document from index
////////////////////////////////////////////////////////////////////////////////
        
TRI_doc_mptr_t* EdgeIndex::remove (TransactionCollection*,
                                   Transaction*,
                                   std::string const& key,
                                   TRI_doc_mptr_t* doc) {
  return nullptr; 
}

////////////////////////////////////////////////////////////////////////////////
/// @brief forget document in index
////////////////////////////////////////////////////////////////////////////////
        
void EdgeIndex::forget (TransactionCollection*,
                        Transaction*, 
                        TRI_doc_mptr_t* doc) {
  WRITE_LOCKER(_lock);

  void* old = _from->remove(doc);

  if (old == nullptr) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_KEYVALUE_KEY_NOT_FOUND);
  }
  
  old = _to->remove(doc);

  if (old == nullptr) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_KEYVALUE_KEY_NOT_FOUND);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief post insert (does nothing)
////////////////////////////////////////////////////////////////////////////////

void EdgeIndex::preCommit (TransactionCollection*, Transaction*) {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief lookup by key with limit, internal function
/// this will dispatch to using either _from, _to, or both
////////////////////////////////////////////////////////////////////////////////

std::vector<TRI_doc_mptr_t*>* EdgeIndex::lookupInternal (Transaction* trans,
                                                         TRI_edge_direction_e direction,
                                                         TRI_edge_header_t const* lookup,
                                                         TRI_doc_mptr_t* previousLast,
                                                         size_t limit) {

  std::unique_ptr<std::vector<TRI_doc_mptr_t*>> theResult(new std::vector<TRI_doc_mptr_t*>);

  if (lookup != nullptr) {
    TRI_ASSERT(previousLast == nullptr);
    if (direction == TRI_EDGE_IN) {
      lookupPart(_to, trans, lookup, theResult.get(), limit, false);
    }
    else if (direction == TRI_EDGE_OUT) {
      lookupPart(_from, trans, lookup, theResult.get(), limit, false);
    }
    else {
      lookupPart(_from, trans, lookup, theResult.get(), limit, false);
      lookupPart(_to, trans, lookup, theResult.get(), limit, true);
    }
  }
  else {
    TRI_ASSERT(previousLast != nullptr);
  }

  return theResult.release();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief lookup by key with limit, internal function
/// this will look in either _from or _to
////////////////////////////////////////////////////////////////////////////////

void EdgeIndex::lookupPart (TRI_EdgeIndexHash_t* part,
                            Transaction* trans,
                            TRI_edge_header_t const* lookup,
                            std::vector<TRI_doc_mptr_t*>* theResult,
                            size_t limit, 
                            bool filterReflexive) {

  bool leave = false;
  while (limit == 0 || theResult->size() < limit) {
    size_t subLimit = (limit == 0) ? 0 : limit - theResult->size();
    std::vector<TRI_doc_mptr_t*>* subResult = part->lookupByKey(lookup, subLimit);

    try {
      for (auto* d : *subResult) {
        if (filterReflexive && IsReflexive(d)) {
          // no need to include this edge
          continue;
        } 

        TransactionId from = d->from();
        TransactionId to = d->to();
        if (trans->isVisibleForRead(from, to)) {
          theResult->push_back(d);
        }
      }
      if (subResult->size() < subLimit) {
        leave = true;
      }
      delete subResult;
    }
    catch (...) {
      delete subResult;
      throw;
    }
    if (subLimit == 0 || leave) {
      break;   // Otherwise we would loop forever
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief lookup by key with limit
////////////////////////////////////////////////////////////////////////////////

std::vector<TRI_doc_mptr_t*>* EdgeIndex::lookup (Transaction* trans,
                                                 TRI_edge_direction_e direction,
                                                 TRI_edge_header_t const* lookup,
                                                 size_t limit = 0) {
  READ_LOCKER(_lock);
  return lookupInternal(trans, direction, lookup, nullptr, limit);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief lookup by key with limit, continuation
////////////////////////////////////////////////////////////////////////////////

std::vector<TRI_doc_mptr_t*>* EdgeIndex::lookupContinue (
                                                  Transaction* trans,
                                                  TRI_edge_direction_e direction,
                                                  TRI_doc_mptr_t* previousLast,
                                                  size_t limit = 0) {

  READ_LOCKER(_lock);
  return lookupInternal(trans, direction, nullptr, previousLast, limit);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief provide a hint for the expected index size
////////////////////////////////////////////////////////////////////////////////

void EdgeIndex::sizeHint (size_t size) {
  WRITE_LOCKER(_lock);

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
  READ_LOCKER(_lock);
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
