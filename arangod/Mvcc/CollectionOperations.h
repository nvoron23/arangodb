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

#ifndef ARANGODB_MVCC_COLLECTION_OPERATIONS_H
#define ARANGODB_MVCC_COLLECTION_OPERATIONS_H 1

#include "Basics/Common.h"
#include "Mvcc/IndexUser.h"
#include "Mvcc/Transaction.h"
#include "Mvcc/TransactionCollection.h"
#include "VocBase/document-collection.h"
#include "VocBase/update-policy.h"
#include "Wal/Logfile.h"

struct TRI_json_t;
struct TRI_shaped_json_s;
struct TRI_shaper_s;

namespace triagens {
  namespace mvcc {

    class TransactionScope;

// -----------------------------------------------------------------------------
// --SECTION--                                                   struct Document
// -----------------------------------------------------------------------------

    struct Document {
      private:

        Document (struct TRI_shaper_s*,
                  struct TRI_shaped_json_s const*,
                  std::string const&,
                  TRI_voc_rid_t,
                  bool,
                  bool);
        
        explicit Document (struct TRI_doc_mptr_t const*);

      public:
        
        Document (Document&&);

        Document (Document const&) = delete;
        Document& operator= (Document&) = delete;

        ~Document ();

////////////////////////////////////////////////////////////////////////////////
/// @brief create a document from JSON
////////////////////////////////////////////////////////////////////////////////

        static Document CreateFromJson (struct TRI_shaper_s*,
                                        struct TRI_json_t const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief create a document from JSON, specifying the key separately
////////////////////////////////////////////////////////////////////////////////

        static Document CreateFromJson (struct TRI_shaper_s*,
                                        struct TRI_json_t const*,
                                        std::string const&);

////////////////////////////////////////////////////////////////////////////////
/// @brief create a document from ShapedJson
////////////////////////////////////////////////////////////////////////////////
        
        static Document CreateFromShapedJson (struct TRI_shaper_s*,
                                              struct TRI_shaped_json_s const*,
                                              char const*,
                                              bool);
        
        static Document CreateFromKey (std::string const&,
                                       TRI_voc_rid_t = 0);
        
        static Document CreateFromMptr (TRI_doc_mptr_t const*);

        struct TRI_shaper_s* shaper;
        struct TRI_shaped_json_s const* shaped;
        std::string key;
        TRI_voc_rid_t revision;
        TRI_document_edge_t const* edge;
        bool keySpecified;
        bool freeShape;
    };

// -----------------------------------------------------------------------------
// --SECTION--                                                struct WriteResult
// -----------------------------------------------------------------------------

    struct WriteResult {
      triagens::wal::Logfile::IdType  logfileId;
      void const*                     mem;
      TRI_voc_tick_t                  tick;
    };

// -----------------------------------------------------------------------------
// --SECTION--                                            struct OperationResult
// -----------------------------------------------------------------------------

    struct OperationResult {
      OperationResult (OperationResult const& other) {
        mptr = other.mptr;
        tick = other.tick;
        actualRevision = other.actualRevision;
        code = other.code;
      }

      OperationResult& operator= (OperationResult const& other) {
        mptr = other.mptr;
        tick = other.tick;
        actualRevision = other.actualRevision;
        code = other.code;
        return *this;
      }

      OperationResult () 
        : mptr(nullptr),
          tick(0),
          actualRevision(0),
          code(TRI_ERROR_NO_ERROR) {
      }
      
      explicit OperationResult (TRI_doc_mptr_t* mptr) 
        : mptr(mptr),
          tick(0),
          actualRevision(0),
          code(TRI_ERROR_NO_ERROR) {
      }
      
      OperationResult (TRI_doc_mptr_t* mptr,
                       TRI_voc_tick_t tick) 
        : mptr(mptr),
          tick(tick),
          actualRevision(0),
          code(TRI_ERROR_NO_ERROR) {
      }
      
      OperationResult (int code, 
                       TRI_voc_rid_t actualRevision) 
        : mptr(nullptr),
          tick(0),
          actualRevision(actualRevision),
          code(code) {
      }
      
      explicit OperationResult (int code)
        : mptr(nullptr),
          tick(0),
          actualRevision(0),
          code(code) {
      }

      TRI_doc_mptr_t* mptr;
      TRI_voc_tick_t  tick;
      TRI_voc_rid_t   actualRevision;
      int             code;
    
    };

// -----------------------------------------------------------------------------
// --SECTION--                                              struct SearchOptions
// -----------------------------------------------------------------------------

    struct SearchOptions {
      int64_t skip    = 0;
      int64_t limit   = INT64_MAX;
      bool    reverse = false;
    };

// -----------------------------------------------------------------------------
// --SECTION--                                           struct OperationOptions
// -----------------------------------------------------------------------------

    struct OperationOptions {
      OperationOptions (int options,
                        TRI_voc_rid_t expectedRevision = 0,
                        TRI_doc_update_policy_e policy = TRI_DOC_UPDATE_LAST_WRITE) 
        : expectedRevision(expectedRevision),
          searchOptions(nullptr),
          policy(policy),
          waitForSync(options & WaitForSync),
          keepNull(options & KeepNull),
          mergeObjects(options & MergeObjects),
          overwrite(options & Overwrite),
          silent(options & Silent) {
      }
      
      OperationOptions () 
        : OperationOptions(0) {
      }

      TRI_voc_rid_t            expectedRevision; // the document revision expected. can be 0 to match any
      SearchOptions*           searchOptions;    // pointer to more specific search options
      TRI_doc_update_policy_e  policy;           // the update policy

      bool                     waitForSync;      // wait for sync?
      bool                     keepNull;         // treat null attributes as to-be-removed?
      bool                     mergeObjects;     // merge object attributes on patch or overwrite?
      bool                     overwrite;        // ignore revision conflicts?
      bool                     silent;           // do not produce output, not used here but by caller

      static int const WaitForSync  = 0x1;
      static int const KeepNull     = 0x2;
      static int const MergeObjects = 0x4;
      static int const Overwrite    = 0x8;
      static int const Silent       = 0x10;
    };
    
// -----------------------------------------------------------------------------
// --SECTION--                                       struct CollectionOperations
// -----------------------------------------------------------------------------

    struct CollectionOperations {

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief return the number of documents in the collection
////////////////////////////////////////////////////////////////////////////////

        static int64_t Count (TransactionScope*,
                              TransactionCollection*);

////////////////////////////////////////////////////////////////////////////////
/// @brief return the size of documents in the collection
////////////////////////////////////////////////////////////////////////////////

        static int64_t Size (TransactionScope*,
                             TransactionCollection*);

////////////////////////////////////////////////////////////////////////////////
/// @brief return the last revision in the collection
////////////////////////////////////////////////////////////////////////////////

        static TRI_voc_rid_t Revision (TransactionScope*,
                                       TransactionCollection*);

////////////////////////////////////////////////////////////////////////////////
/// @brief reads a random document from the collection
////////////////////////////////////////////////////////////////////////////////

        static OperationResult RandomDocument (TransactionScope*,
                                               TransactionCollection*);

////////////////////////////////////////////////////////////////////////////////
/// @brief truncate the collection
////////////////////////////////////////////////////////////////////////////////

        static OperationResult Truncate (TransactionScope*, 
                                         TransactionCollection*,
                                         OperationOptions const&);

////////////////////////////////////////////////////////////////////////////////
/// @brief reads a random document from the collection
////////////////////////////////////////////////////////////////////////////////

        static OperationResult ReadDocument (TransactionScope*, 
                                             TransactionCollection*); 

////////////////////////////////////////////////////////////////////////////////
/// @brief insert a document
////////////////////////////////////////////////////////////////////////////////

        static OperationResult InsertDocument (TransactionScope*, 
                                               TransactionCollection*, 
                                               Document&, 
                                               OperationOptions const&);

////////////////////////////////////////////////////////////////////////////////
/// @brief read a document by its key
////////////////////////////////////////////////////////////////////////////////

        static OperationResult ReadDocument (TransactionScope*, 
                                             TransactionCollection*, 
                                             Document const&, 
                                             OperationOptions const&);

////////////////////////////////////////////////////////////////////////////////
/// @brief read all documents of the collection
////////////////////////////////////////////////////////////////////////////////

        static OperationResult ReadAllDocuments (TransactionScope*, 
                                                 TransactionCollection*,
                                                 std::vector<TRI_doc_mptr_t const*>&, 
                                                 OperationOptions const&);

////////////////////////////////////////////////////////////////////////////////
/// @brief read all documents by example
////////////////////////////////////////////////////////////////////////////////

        static OperationResult ReadByExample (TransactionScope*, 
                                              TransactionCollection*,
                                              std::vector<TRI_shape_pid_t> const&,
                                              std::vector<TRI_shaped_json_t*> const&,
                                              std::vector<TRI_doc_mptr_t const*>&, 
                                              OperationOptions const&);

////////////////////////////////////////////////////////////////////////////////
/// @brief remove a document
////////////////////////////////////////////////////////////////////////////////

        static OperationResult RemoveDocument (TransactionScope*, 
                                               TransactionCollection*,
                                               Document const&,
                                               OperationOptions const&);

////////////////////////////////////////////////////////////////////////////////
/// @brief update a document
////////////////////////////////////////////////////////////////////////////////

        static OperationResult UpdateDocument (TransactionScope*, 
                                               TransactionCollection*,
                                               Document&,
                                               TRI_json_t const* updateDocument,
                                               OperationOptions const&);

////////////////////////////////////////////////////////////////////////////////
/// @brief replace a document
////////////////////////////////////////////////////////////////////////////////

        static OperationResult ReplaceDocument (TransactionScope*, 
                                                TransactionCollection*,
                                                Document&,
                                                OperationOptions const&);

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief insert a document
////////////////////////////////////////////////////////////////////////////////

        static OperationResult InsertDocumentWorker (TransactionScope*, 
                                                     TransactionCollection*, 
                                                     IndexUser&,
                                                     Document&, 
                                                     OperationOptions const&);

////////////////////////////////////////////////////////////////////////////////
/// @brief read a document by its key
////////////////////////////////////////////////////////////////////////////////

        static OperationResult ReadDocumentWorker (TransactionScope*, 
                                                   TransactionCollection*, 
                                                   IndexUser&,
                                                   Document const&, 
                                                   OperationOptions const&);

////////////////////////////////////////////////////////////////////////////////
/// @brief remove a document
////////////////////////////////////////////////////////////////////////////////

        static OperationResult RemoveDocumentWorker (TransactionScope*, 
                                                     TransactionCollection*,
                                                     IndexUser&,
                                                     Document const&,
                                                     OperationOptions const&,
                                                     TransactionId&);

////////////////////////////////////////////////////////////////////////////////
/// @brief validate the revision of the document found
////////////////////////////////////////////////////////////////////////////////
    
        static int CheckRevision (TRI_doc_mptr_t const*,
                                  Document const&,
                                  TRI_doc_update_policy_e,
                                  TRI_voc_rid_t* = nullptr);

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a key if not specified, or validates it if specified
/// will throw if the key is invalid
////////////////////////////////////////////////////////////////////////////////

        static void CreateOrValidateKey (TransactionCollection*,
                                         Document*);                   

////////////////////////////////////////////////////////////////////////////////
/// @brief write a document or remove marker into the write-ahead log
////////////////////////////////////////////////////////////////////////////////

        static int WriteMarker (triagens::wal::Marker const*,
                                TransactionCollection*,
                                WriteResult&);

      public:
       
        static int const KeySpecified = true;
        static int const NoKeySpecified = false;
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
