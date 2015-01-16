////////////////////////////////////////////////////////////////////////////////
/// @brief edge collection functionality
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
/// @author Jan Steemann
/// @author Copyright 2014, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "edge-collection.h"

#include "Basics/AssocMulti.h"
#include "Basics/logging.h"

#include "VocBase/document-collection.h"
#include "VocBase/index.h"

// -----------------------------------------------------------------------------
// --SECTION--                                                       EDGES INDEX
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                 private functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief find the edges index of a document collection
////////////////////////////////////////////////////////////////////////////////

static TRI_edge_index_t* FindEdgesIndex (TRI_document_collection_t const* document) {
  if (document->_info._type == TRI_COL_TYPE_EDGE) {

    for (auto idx : document->_allIndexes) {
      if (idx->_type == TRI_IDX_TYPE_EDGE_INDEX) {
        TRI_edge_index_t* edgesIndex = (TRI_edge_index_t*) idx;
        return edgesIndex;
      }
    }
  }

  // collection does not have an edges index... caller must handle that
  return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief check whether the _from and _to end of an edge are identical
////////////////////////////////////////////////////////////////////////////////

static bool IsReflexive (TRI_doc_mptr_t const* mptr) {
  TRI_df_marker_t const* marker = static_cast<TRI_df_marker_t const*>(mptr->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

  if (marker->_type == TRI_DOC_MARKER_KEY_EDGE) {
    TRI_doc_edge_key_marker_t const* edge = static_cast<TRI_doc_edge_key_marker_t const*>(mptr->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

    if (edge->_toCid == edge->_fromCid) {
      char const* fromKey = reinterpret_cast<char const*>(edge) + edge->_offsetFromKey;
      char const* toKey =   reinterpret_cast<char const*>(edge) + edge->_offsetToKey;

      return strcmp(fromKey, toKey) == 0;
    }
  }
  else if (marker->_type == TRI_WAL_MARKER_EDGE) {
    triagens::wal::edge_marker_t const* edge = static_cast<triagens::wal::edge_marker_t const*>(mptr->getDataPtr());  // ONLY IN INDEX, PROTECTED by RUNTIME

    if (edge->_toCid == edge->_fromCid) {
      char const* fromKey = reinterpret_cast<char const*>(edge) + edge->_offsetFromKey;
      char const* toKey =   reinterpret_cast<char const*>(edge) + edge->_offsetToKey;

      return strcmp(fromKey, toKey) == 0;
    }
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief find edges matching search criteria and add them to the result
/// this function is called two times for each edge query:
/// the first call (with matchType 1) will query the index with the originally
/// requested direction, whereas the second call will query the index with the
/// opposite direction (with matchType 2 or 3) to find all counterparts.
/// Return value is an error code.
////////////////////////////////////////////////////////////////////////////////

static int FindEdges (TRI_edge_direction_e direction,
                      TRI_edge_index_t* idx,
                      std::vector<TRI_doc_mptr_copy_t>& result,
                      TRI_edge_header_t* entry,
                      int matchType) {
  std::vector<void*>* found = nullptr;

  if (direction == TRI_EDGE_OUT) {
    try {
      found = idx->_edges_from->lookupByKey(entry);
    }
    catch (...) {
      return TRI_ERROR_OUT_OF_MEMORY;
    }
  }
  else if (direction == TRI_EDGE_IN) {
    try {
      found = idx->_edges_to->lookupByKey(entry);
    }
    catch (...) {
      return TRI_ERROR_OUT_OF_MEMORY;
    }
  }
  else {
    TRI_ASSERT(false);   // TRI_EDGE_ANY not supported here
  }

  size_t const n = found->size();

  if (n > 0) {
    if (result.capacity() == 0) {
      // if result vector is still empty and we have results, re-init the
      // result vector to a "good" size. this will save later reallocations
      result.reserve(n);
    }

    // add all results found
    for (size_t i = 0;  i < n;  ++i) {
      TRI_doc_mptr_t* edge = (TRI_doc_mptr_t*) found->at(i);

      // the following queries will use the following sequences of matchTypes:
      // inEdges(): 1,  outEdges(): 1,  edges(): 1, 3

      // if matchType is 1, we'll return all found edges without filtering
      // We'll exclude all loop edges now (we already got them in iteration 1),
      // and alsoexclude all unidirectional edges
      //
      // if matchType is 3, the direction is also reversed. We'll exclude all
      // loop edges now (we already got them in iteration 1)

      if (matchType > 1) {

        // if the edge is a loop, we have already found it in iteration 1
        // we must skip it here, otherwise we would produce duplicates
        if (IsReflexive(edge)) {
          continue;
        }
      }

      result.push_back(*edge);

    }
  }

  delete found;

  return TRI_ERROR_NO_ERROR;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief looks up edges
////////////////////////////////////////////////////////////////////////////////

std::vector<TRI_doc_mptr_copy_t> TRI_LookupEdgesDocumentCollection (
                                        TRI_document_collection_t* document,
                                        TRI_edge_direction_e direction,
                                        TRI_voc_cid_t cid,
                                        TRI_voc_key_t key) {
  // search criteria
  TRI_edge_header_t entry;
  entry._cid = cid;
  entry._key = key;

  // initialise the result vector
  std::vector<TRI_doc_mptr_copy_t> result;

  TRI_edge_index_t* edgesIndex = FindEdgesIndex(document);

  if (edgesIndex == nullptr) {
    LOG_ERROR("collection does not have an edges index");
    return result;
  }

  if (direction == TRI_EDGE_IN) {
    // get all edges with a matching IN vertex
    if (FindEdges(TRI_EDGE_IN, edgesIndex, result, &entry, 1) !=
        TRI_ERROR_NO_ERROR) {
      result.clear();  // there is currently no way to report the error
      return result;
    }
  }
  else if (direction == TRI_EDGE_OUT) {
    // get all edges with a matching OUT vertex
    if (FindEdges(TRI_EDGE_OUT, edgesIndex, result, &entry, 1) !=
        TRI_ERROR_NO_ERROR) {
      result.clear();
      return result;
    }
  }
  else if (direction == TRI_EDGE_ANY) {
    // get all edges with a matching IN vertex
    if (FindEdges(TRI_EDGE_IN, edgesIndex, result, &entry, 1) != 
        TRI_ERROR_NO_ERROR) {
      result.clear();
      return result;
    }
    // add all non-reflexive edges with a matching OUT vertex
    if (FindEdges(TRI_EDGE_OUT, edgesIndex, result, &entry, 3) !=
        TRI_ERROR_NO_ERROR) {
      result.clear();
      return result;
    }
  }

  return result;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
