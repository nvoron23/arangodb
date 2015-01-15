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
                  char const*,
                  bool);

      public:
        
        Document (Document&&);

        Document (Document const&) = delete;
        Document& operator= (Document&) = delete;

        ~Document ();

        static Document createFromJson (struct TRI_shaper_s*,
                                        struct TRI_json_t const*);
        
        static Document createFromShapedJson (struct TRI_shaper_s*,
                                              struct TRI_shaped_json_s const*,
                                              char const* key = nullptr);

        struct TRI_shaper_s* shaper;
        struct TRI_shaped_json_s const* shaped;
        char const* key;
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
        : code(TRI_ERROR_NO_ERROR) {
      }

      TRI_doc_mptr_copy_t document;
      int code;
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

        static OperationResult insertDocument (Transaction&, 
                                               TransactionCollection&, 
                                               Document&, 
                                               OperationOptions&);

////////////////////////////////////////////////////////////////////////////////
/// @brief remove a document
////////////////////////////////////////////////////////////////////////////////

        static OperationResult removeDocument (Transaction&, 
                                               TransactionCollection&,
                                               Document&,
                                               OperationOptions&);

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a key if not specified, or validates it if specified
/// returns the key
////////////////////////////////////////////////////////////////////////////////

      private:

        static std::string createOrValidateKey (TransactionCollection&,
                                                Document&);                   

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
