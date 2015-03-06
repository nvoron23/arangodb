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
#include "VocBase/voc-types.h"

// -----------------------------------------------------------------------------
// --SECTION--                                              forward declarations
// -----------------------------------------------------------------------------

struct TRI_json_t;
struct TRI_document_collection_t;

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
/// @brief saves an index
////////////////////////////////////////////////////////////////////////////////

int TRI_SaveIndex (struct TRI_document_collection_t*,
                   triagens::mvcc::Index const*,
                   bool);

////////////////////////////////////////////////////////////////////////////////
/// @brief index comparator, used by the coordinator to detect if two index
/// contents are the same
////////////////////////////////////////////////////////////////////////////////

bool IndexComparator (struct TRI_json_t const*, 
                      struct TRI_json_t const*);

#endif

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
