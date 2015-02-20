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

#ifndef ARANGODB_MVCC_OPEN_ITERATOR_H
#define ARANGODB_MVCC_OPEN_ITERATOR_H 1

#include "Basics/Common.h"
#include "VocBase/datafile.h"
#include "VocBase/voc-types.h"

struct TRI_document_collection_t;
struct TRI_doc_mptr_t;
struct TRI_doc_datafile_info_s;

namespace triagens {
  namespace mvcc {

    class PrimaryIndex;
    class Transaction;
    class TransactionCollection;

// -----------------------------------------------------------------------------
// --SECTION--                                          struct OpenIteratorState
// -----------------------------------------------------------------------------

    struct OpenIteratorState {

      public:
        
        OpenIteratorState (OpenIteratorState const&) = delete;
        OpenIteratorState& operator= (OpenIteratorState const&) = delete;

        OpenIteratorState (struct TRI_document_collection_t*);

        ~OpenIteratorState ();

        struct TRI_doc_mptr_t* lookupDocument (char const*);
        struct TRI_doc_mptr_t* removeDocument (char const*);
        void insertDocument (TRI_df_marker_t const*);
        void trackRevisionId (TRI_voc_rid_t);
        void ensureDatafileId (TRI_voc_fid_t);
        void addShape (TRI_df_marker_t const*);
        void addAttribute (TRI_df_marker_t const*);
        void addDeletion ();
        void addDead (TRI_df_marker_t const*);
        void addAlive (TRI_df_marker_t const*);

        struct TRI_document_collection_t* collection;
        TRI_voc_fid_t datafileId;
        TRI_voc_rid_t revisionId;
        int64_t documentCount;
        int64_t documentSize;
        struct TRI_doc_datafile_info_s* dfi;
        triagens::mvcc::PrimaryIndex* primaryIndex;
    };

// -----------------------------------------------------------------------------
// --SECTION--                                                class OpenIterator
// -----------------------------------------------------------------------------

    class OpenIterator {

      private:

        OpenIterator (OpenIterator const&) = delete;
        OpenIterator& operator= (OpenIterator const&) = delete;
        OpenIterator () = delete;

      public:

        static bool execute (TRI_df_marker_t const*,
                             void* data,
                             TRI_datafile_t*);

      private:

        static int handleDocumentMarker (TRI_df_marker_t const*,
                                         TRI_datafile_t*,
                                         OpenIteratorState*);

        static int handleDeletionMarker (TRI_df_marker_t const*,
                                         TRI_datafile_t*,
                                         OpenIteratorState*);

        static int handleShapeMarker (TRI_df_marker_t const*,
                                      TRI_datafile_t*,
                                      OpenIteratorState*);
        
        static int handleAttributeMarker (TRI_df_marker_t const*,
                                          TRI_datafile_t*,
                                          OpenIteratorState*);

    };

  }

}

#endif

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
