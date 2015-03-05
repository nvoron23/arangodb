////////////////////////////////////////////////////////////////////////////////
/// @brief document request handler
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
/// @author Copyright 2010-2014, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGODB_REST_MVCC_HANDLER_REST_DOCUMENT_HANDLER_H
#define ARANGODB_REST_MVCC_HANDLER_REST_DOCUMENT_HANDLER_H 1

#include "Basics/Common.h"
#include "RestHandler/RestVocbaseBaseHandler.h"

struct TRI_doc_mptr_t;

namespace triagens {
  namespace mvcc {
    struct OperationResult;
    class Transaction;
    class TransactionCollection;
  }

  namespace arango {

// -----------------------------------------------------------------------------
// --SECTION--                                     class RestMvccDocumentHandler
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief document request handler
////////////////////////////////////////////////////////////////////////////////

    class RestMvccDocumentHandler : public RestVocbaseBaseHandler {

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief constructor
////////////////////////////////////////////////////////////////////////////////

        RestMvccDocumentHandler (rest::HttpRequest*);

// -----------------------------------------------------------------------------
// --SECTION--                                                   Handler methods
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        status_t execute () override final;

// -----------------------------------------------------------------------------
// --SECTION--                                                 protected methods
// -----------------------------------------------------------------------------

      protected:

////////////////////////////////////////////////////////////////////////////////
/// @brief get collection type
////////////////////////////////////////////////////////////////////////////////

        virtual TRI_col_type_e getCollectionType () const {
          return TRI_COL_TYPE_DOCUMENT;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief inserts a document
////////////////////////////////////////////////////////////////////////////////

        virtual bool insertDocument ();

////////////////////////////////////////////////////////////////////////////////
/// @brief reads a single or all documents
////////////////////////////////////////////////////////////////////////////////

        bool readDocument ();

////////////////////////////////////////////////////////////////////////////////
/// @brief reads a single document
////////////////////////////////////////////////////////////////////////////////

        bool readSingleDocument (bool generateBody);

////////////////////////////////////////////////////////////////////////////////
/// @brief reads all documents
////////////////////////////////////////////////////////////////////////////////

        bool readAllDocuments ();

////////////////////////////////////////////////////////////////////////////////
/// @brief reads a single document head
////////////////////////////////////////////////////////////////////////////////

        bool checkDocument ();

////////////////////////////////////////////////////////////////////////////////
/// @brief replaces a document
////////////////////////////////////////////////////////////////////////////////

        bool replaceDocument ();

////////////////////////////////////////////////////////////////////////////////
/// @brief updates a document
////////////////////////////////////////////////////////////////////////////////

        bool updateDocument ();

////////////////////////////////////////////////////////////////////////////////
/// @brief helper function for replace and update
////////////////////////////////////////////////////////////////////////////////

        bool modifyDocument (bool);
 
////////////////////////////////////////////////////////////////////////////////
/// @brief removes a document
////////////////////////////////////////////////////////////////////////////////

        bool removeDocument ();

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a document, coordinator case in a cluster
////////////////////////////////////////////////////////////////////////////////

        bool insertDocumentCoordinator (char const* collection,
                                        bool waitForSync,
                                        TRI_json_t* json);

////////////////////////////////////////////////////////////////////////////////
/// @brief delete a document, coordinator case in a cluster
////////////////////////////////////////////////////////////////////////////////

        bool removeDocumentCoordinator (std::string const& collname,
                                        std::string const& key,
                                        TRI_voc_rid_t const rev,
                                        TRI_doc_update_policy_e policy,
                                        bool waitForSync);

////////////////////////////////////////////////////////////////////////////////
/// @brief read a single document, coordinator case in a cluster
////////////////////////////////////////////////////////////////////////////////

        bool getDocumentCoordinator (std::string const& collname,
                                     std::string const& key,
                                     bool generateBody);

////////////////////////////////////////////////////////////////////////////////
/// @brief read all documents, coordinator case in a cluster
////////////////////////////////////////////////////////////////////////////////

        bool getAllDocumentsCoordinator (std::string const& collname,
                                         std::string const& returnType);

////////////////////////////////////////////////////////////////////////////////
/// @brief read a single document, coordinator case in a cluster
////////////////////////////////////////////////////////////////////////////////

        bool modifyDocumentCoordinator (std::string const& collname,
                                        std::string const& key,
                                        TRI_voc_rid_t const rev,
                                        TRI_doc_update_policy_e policy,
                                        bool waitForSync,
                                        bool isPatch,
                                        TRI_json_t* json);

////////////////////////////////////////////////////////////////////////////////
/// @brief generates a HTTP 201 or 202 response
////////////////////////////////////////////////////////////////////////////////

        void generate20x (triagens::rest::HttpResponse::HttpResponseCode,
                          triagens::mvcc::TransactionCollection*,
                          triagens::mvcc::OperationResult const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief generates a 304 not modified response
////////////////////////////////////////////////////////////////////////////////
        
        void generateNotModified (TRI_voc_rid_t);

////////////////////////////////////////////////////////////////////////////////
/// @brief generates a document response
////////////////////////////////////////////////////////////////////////////////

        void generateDocument (triagens::mvcc::Transaction*,
                               triagens::mvcc::TransactionCollection*,
                               triagens::mvcc::OperationResult const*,
                               bool);

////////////////////////////////////////////////////////////////////////////////
/// @brief generates a HTTP 412 response
////////////////////////////////////////////////////////////////////////////////

        void generatePreconditionFailed (triagens::mvcc::TransactionCollection*,
                                         triagens::mvcc::OperationResult const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief generates a HTTP 201 or 202 response, based on waitForSync flag
////////////////////////////////////////////////////////////////////////////////

        triagens::rest::HttpResponse::HttpResponseCode insertResponseCode (bool) const;

////////////////////////////////////////////////////////////////////////////////
/// @brief generates a HTTP 200 or 202 response, based on waitForSync flag
////////////////////////////////////////////////////////////////////////////////

        triagens::rest::HttpResponse::HttpResponseCode removeResponseCode (bool) const;

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
