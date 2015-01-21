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
#include "Mvcc/Transaction.h"
#include "Mvcc/TransactionCollection.h"
#include "VocBase/document-collection.h"
#include "Wal/Logfile.h"

struct TRI_json_t;
struct TRI_shaped_json_s;
struct TRI_shaper_s;

namespace triagens {
  namespace mvcc {

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

      public:
        
        Document (Document&&);

        Document (Document const&) = delete;
        Document& operator= (Document&) = delete;

        ~Document ();

        static Document CreateFromJson (struct TRI_shaper_s*,
                                        struct TRI_json_t const*);
        
        static Document CreateFromShapedJson (struct TRI_shaper_s*,
                                              struct TRI_shaped_json_s const*,
                                              char const* = nullptr);

        struct TRI_shaper_s* shaper;
        struct TRI_shaped_json_s const* shaped;
        std::string key;
        TRI_voc_rid_t revision;
        bool keySpecified;
        bool freeShape;
    };

// -----------------------------------------------------------------------------
// --SECTION--                                            struct OperationResult
// -----------------------------------------------------------------------------

    struct OperationResult {
      // TODO: implement copy ctor & copy assignment ctor
      OperationResult (OperationResult const& other) {
        // TODO: implement
        // ...
      }

      OperationResult& operator= (OperationResult const& other) {
        // TODO: implement
        return *this;
      }

      OperationResult () 
        : mptr(),
          logfileId(0),
          tick(0),
          data(nullptr),
          code(TRI_ERROR_NO_ERROR) {
      }

      TRI_doc_mptr_copy_t            mptr;
      triagens::wal::Logfile::IdType logfileId;
      TRI_voc_tick_t                 tick;
      void const*                    data;
      int                            code;
    
    };

// -----------------------------------------------------------------------------
// --SECTION--                                           struct OperationOptions
// -----------------------------------------------------------------------------

    struct OperationOptions {
      OperationOptions (int options,
                        TRI_voc_rid_t expectedRevision = 0) 
        : expectedRevision(expectedRevision),
          waitForSync(options & WaitForSync),
          keepNull(options & KeepNull),
          mergeObjects(options & MergeObjects),
          overwrite(options & Overwrite) {
      }
      
      OperationOptions () 
        : OperationOptions(0) {
      }

      TRI_voc_rid_t const expectedRevision;

      bool const waitForSync;      // wait for sync?
      bool const keepNull;         // treat null attributes as to-be-removed?
      bool const mergeObjects;     // merge object attributes on patch or overwrite?
      bool const overwrite;        // ignore revision conflicts?

      static int const WaitForSync  = 0x1;
      static int const KeepNull     = 0x2;
      static int const MergeObjects = 0x4;
      static int const Overwrite    = 0x8;
    };
    
// -----------------------------------------------------------------------------
// --SECTION--                                       struct CollectionOperations
// -----------------------------------------------------------------------------

    struct CollectionOperations {

////////////////////////////////////////////////////////////////////////////////
/// @brief insert a document
////////////////////////////////////////////////////////////////////////////////

        static OperationResult InsertDocument (Transaction&, 
                                               TransactionCollection&, 
                                               Document&, 
                                               OperationOptions&);

////////////////////////////////////////////////////////////////////////////////
/// @brief remove a document
////////////////////////////////////////////////////////////////////////////////

        static OperationResult RemoveDocument (Transaction&, 
                                               TransactionCollection&,
                                               Document&,
                                               OperationOptions&);

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a key if not specified, or validates it if specified
/// will throw if the key is invalid
////////////////////////////////////////////////////////////////////////////////

        static void CreateOrValidateKey (TransactionCollection&,
                                         Document&);                   

        static int WriteMarker (triagens::wal::Marker*,
                                TransactionCollection&,
                                OperationResult&);

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
