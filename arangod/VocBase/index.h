////////////////////////////////////////////////////////////////////////////////
/// @brief index
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

#ifndef ARANGODB_VOC_BASE_INDEX_H
#define ARANGODB_VOC_BASE_INDEX_H 1

#include "Basics/Common.h"
#include "Basics/AssocMulti.h"
#include "Basics/json.h"
#include "IndexOperators/index-operator.h"
#include "ShapedJson/shaped-json.h"
#include "VocBase/voc-types.h"
#include "VocBase/vocbase.h"

// -----------------------------------------------------------------------------
// --SECTION--                                              forward declarations
// -----------------------------------------------------------------------------

struct TRI_collection_t;
struct TRI_doc_mptr_t;
struct TRI_shaped_json_s;
struct TRI_document_collection_t;
struct TRI_transaction_collection_s;

namespace triagens {
  namespace mvcc {
    class Index;
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                      public types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief index type
////////////////////////////////////////////////////////////////////////////////

typedef enum {
  TRI_IDX_TYPE_UNKNOWN = 0,
  TRI_IDX_TYPE_PRIMARY_INDEX,
  TRI_IDX_TYPE_GEO1_INDEX,
  TRI_IDX_TYPE_GEO2_INDEX,
  TRI_IDX_TYPE_HASH_INDEX,
  TRI_IDX_TYPE_EDGE_INDEX,
  TRI_IDX_TYPE_FULLTEXT_INDEX,
  TRI_IDX_TYPE_PRIORITY_QUEUE_INDEX, // DEPRECATED and not functional anymore
  TRI_IDX_TYPE_SKIPLIST_INDEX,
  TRI_IDX_TYPE_BITARRAY_INDEX,       // DEPRECATED and not functional anymore
  TRI_IDX_TYPE_CAP_CONSTRAINT
}
TRI_idx_type_e;

////////////////////////////////////////////////////////////////////////////////
/// @brief geo index variants
////////////////////////////////////////////////////////////////////////////////

typedef enum {
  INDEX_GEO_NONE = 0,
  INDEX_GEO_INDIVIDUAL_LAT_LON,
  INDEX_GEO_COMBINED_LAT_LON,
  INDEX_GEO_COMBINED_LON_LAT
}
TRI_index_geo_variant_e;

////////////////////////////////////////////////////////////////////////////////
/// @brief index base class
////////////////////////////////////////////////////////////////////////////////

typedef struct TRI_index_s {
  TRI_idx_iid_t   _iid;
  TRI_idx_type_e  _type;
  struct TRI_document_collection_t* _collection;

  TRI_vector_string_t _fields;
  bool _unique;
  bool _sparse;
  bool _hasSelectivityEstimate;

  double (*selectivityEstimate) (struct TRI_index_s const*);
  size_t (*memory) (struct TRI_index_s const*);
  TRI_json_t* (*json) (struct TRI_index_s const*);

  // .........................................................................................
  // the following functions are called for document/collection administration
  // .........................................................................................

  int (*insert) (struct TRI_index_s*, struct TRI_doc_mptr_t const*, bool);
  int (*remove) (struct TRI_index_s*, struct TRI_doc_mptr_t const*, bool);

  // NULL by default. will only be called if non-NULL
  int (*postInsert) (struct TRI_transaction_collection_s*, struct TRI_index_s*, struct TRI_doc_mptr_t const*);

  // a garbage collection function for the index
  int (*cleanup) (struct TRI_index_s*);

  // give index a hint about the expected size
  int (*sizeHint) (struct TRI_index_s*, size_t);

  // .........................................................................................
  // the following functions are called by the query machinery which attempting to determine an
  // appropriate index and when using the index to obtain a result set.
  // .........................................................................................
}
TRI_index_t;

////////////////////////////////////////////////////////////////////////////////
/// @brief edge index
////////////////////////////////////////////////////////////////////////////////

typedef triagens::basics::AssocMulti<void, void, uint32_t> TRI_EdgeIndexHash_t;

typedef struct TRI_edge_index_s {
  TRI_index_t base;

  TRI_EdgeIndexHash_t* _edges_from;
  TRI_EdgeIndexHash_t* _edges_to;
}
TRI_edge_index_t;

// -----------------------------------------------------------------------------
// --SECTION--                                                             INDEX
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief removes an index file
////////////////////////////////////////////////////////////////////////////////

int TRI_RemoveIndexFile (struct TRI_document_collection_t*,
                         TRI_idx_iid_t);

////////////////////////////////////////////////////////////////////////////////
/// @brief removes an index file
////////////////////////////////////////////////////////////////////////////////

bool TRI_RemoveIndexFile (struct TRI_document_collection_t*,
                          TRI_index_t*);

////////////////////////////////////////////////////////////////////////////////
/// @brief saves an index
////////////////////////////////////////////////////////////////////////////////

int TRI_SaveIndex (struct TRI_document_collection_t*,
                   triagens::mvcc::Index const*,
                   bool);

////////////////////////////////////////////////////////////////////////////////
/// @brief saves an index
////////////////////////////////////////////////////////////////////////////////

int TRI_SaveIndex (struct TRI_document_collection_t*,
                   TRI_index_t*,
                   bool);

////////////////////////////////////////////////////////////////////////////////
/// @brief index comparator, used by the coordinator to detect if two index
/// contents are the same
////////////////////////////////////////////////////////////////////////////////

bool IndexComparator (TRI_json_t const* lhs, TRI_json_t const* rhs);

#endif

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
