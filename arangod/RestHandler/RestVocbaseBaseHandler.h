////////////////////////////////////////////////////////////////////////////////
/// @brief abstract base request handler
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
/// @author Copyright 2010-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGODB_REST_HANDLER_REST_VOCBASE_BASE_HANDLER_H
#define ARANGODB_REST_HANDLER_REST_VOCBASE_BASE_HANDLER_H 1

#include "Basics/Common.h"
#include "Admin/RestBaseHandler.h"
#include "Basics/json.h"
#include "VocBase/collection.h"
#include "VocBase/update-policy.h"
#include "VocBase/voc-types.h"

// -----------------------------------------------------------------------------
// --SECTION--                                              forward declarations
// -----------------------------------------------------------------------------

struct TRI_document_collection_t;
struct TRI_vocbase_col_s;
struct TRI_vocbase_s;

// -----------------------------------------------------------------------------
// --SECTION--                                      class RestVocbaseBaseHandler
// -----------------------------------------------------------------------------

namespace triagens {
  namespace rest {
    class HttpResponse;
  }

  namespace arango {

    class CollectionNameResolver;
    class VocbaseContext;

////////////////////////////////////////////////////////////////////////////////
/// @brief abstract base request handler
////////////////////////////////////////////////////////////////////////////////

    class RestVocbaseBaseHandler : public admin::RestBaseHandler {
      private:
        RestVocbaseBaseHandler (RestVocbaseBaseHandler const&) = delete;
        RestVocbaseBaseHandler& operator= (RestVocbaseBaseHandler const&) = delete;

// -----------------------------------------------------------------------------
// --SECTION--                                                  public constants
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief batch path
////////////////////////////////////////////////////////////////////////////////

        static const std::string BATCH_PATH;

////////////////////////////////////////////////////////////////////////////////
/// @brief document path
////////////////////////////////////////////////////////////////////////////////

        static const std::string DOCUMENT_PATH;

////////////////////////////////////////////////////////////////////////////////
/// @brief document import path
////////////////////////////////////////////////////////////////////////////////

        static const std::string IMPORT_PATH;

////////////////////////////////////////////////////////////////////////////////
/// @brief edge path
////////////////////////////////////////////////////////////////////////////////

        static const std::string EDGE_PATH;

////////////////////////////////////////////////////////////////////////////////
/// @brief replication path
////////////////////////////////////////////////////////////////////////////////

        static const std::string REPLICATION_PATH;

////////////////////////////////////////////////////////////////////////////////
/// @brief upload path
////////////////////////////////////////////////////////////////////////////////

        static const std::string UPLOAD_PATH;

////////////////////////////////////////////////////////////////////////////////
/// @brief name of the queue
////////////////////////////////////////////////////////////////////////////////

        static const std::string QUEUE_NAME;

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief constructor
////////////////////////////////////////////////////////////////////////////////

        RestVocbaseBaseHandler (rest::HttpRequest*);

////////////////////////////////////////////////////////////////////////////////
/// @brief destructor
////////////////////////////////////////////////////////////////////////////////

        ~RestVocbaseBaseHandler ();

// -----------------------------------------------------------------------------
// --SECTION--                                                 protected methods
// -----------------------------------------------------------------------------

      protected:

////////////////////////////////////////////////////////////////////////////////
/// @brief return a boolean value from the request
////////////////////////////////////////////////////////////////////////////////

        bool extractBoolValue (char const*,
                               bool) const;

////////////////////////////////////////////////////////////////////////////////
/// @brief check if a collection needs to be created on the fly
///
/// this method will check the "createCollection" attribute of the request. if
/// it is set to true, it will verify that the named collection actually exists.
/// if the collection does not yet exist, it will create it on the fly.
/// if the "createCollection" attribute is not set or set to false, nothing will
/// happen, and the collection name will not be checked
////////////////////////////////////////////////////////////////////////////////

        bool checkCreateCollection (std::string const&,
                                    TRI_col_type_e);

////////////////////////////////////////////////////////////////////////////////
/// @brief extracts the revision
///
/// @note @FA{header} must be lowercase.
////////////////////////////////////////////////////////////////////////////////

        TRI_voc_rid_t extractRevision (char const*,
                                       char const*,
                                       bool&);

////////////////////////////////////////////////////////////////////////////////
/// @brief extracts the update policy
////////////////////////////////////////////////////////////////////////////////

        TRI_doc_update_policy_e extractUpdatePolicy () const;

////////////////////////////////////////////////////////////////////////////////
/// @brief parses the body
////////////////////////////////////////////////////////////////////////////////

        TRI_json_t* parseJsonBody ();

////////////////////////////////////////////////////////////////////////////////
/// @brief generates not implemented
////////////////////////////////////////////////////////////////////////////////

        void generateNotImplemented (std::string const&);

////////////////////////////////////////////////////////////////////////////////
/// @brief parses a document handle, on a cluster this will parse the
/// collection name as a cluster-wide collection name and return a
/// cluster-wide collection ID in `cid`.
////////////////////////////////////////////////////////////////////////////////

        int parseDocumentId (triagens::arango::CollectionNameResolver const*,
                             std::string const&,
                             TRI_voc_cid_t&,
                             std::unique_ptr<char[]>&);

// -----------------------------------------------------------------------------
// --SECTION--                                               protected variables
// -----------------------------------------------------------------------------

      protected:

////////////////////////////////////////////////////////////////////////////////
/// @brief request context
////////////////////////////////////////////////////////////////////////////////

        VocbaseContext* _context;

////////////////////////////////////////////////////////////////////////////////
/// @brief the vocbase
////////////////////////////////////////////////////////////////////////////////

        struct TRI_vocbase_s* _vocbase;

// -----------------------------------------------------------------------------
// --SECTION--                                                   Handler methods
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        bool isDirect () override {
          return false;
        }

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        std::string const& queue () const override;

////////////////////////////////////////////////////////////////////////////////
/// @brief prepareExecute, to react to X-Arango-Nolock header
////////////////////////////////////////////////////////////////////////////////

        virtual void prepareExecute ();

////////////////////////////////////////////////////////////////////////////////
/// @brief finalizeExecute, to react to X-Arango-Nolock header
////////////////////////////////////////////////////////////////////////////////

        virtual void finalizeExecute ();

////////////////////////////////////////////////////////////////////////////////
/// @brief _nolockHeaderFound
////////////////////////////////////////////////////////////////////////////////

        bool _nolockHeaderFound;

////////////////////////////////////////////////////////////////////////////////
/// @brief _nolockHeaderFound
////////////////////////////////////////////////////////////////////////////////

        std::unordered_set<std::string>* _nolockHeaderSet;

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
