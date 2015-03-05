////////////////////////////////////////////////////////////////////////////////
/// @brief import request handler
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

#ifndef ARANGODB_REST_HANDLER_REST_MVCC_IMPORT_HANDLER_H
#define ARANGODB_REST_HANDLER_REST_MVCC_IMPORT_HANDLER_H 1

#include "Basics/Common.h"
#include "Mvcc/CollectionOperations.h"
#include "RestHandler/RestVocbaseBaseHandler.h"

// -----------------------------------------------------------------------------
// --SECTION--                                             RestMvccImportHandler
// -----------------------------------------------------------------------------

namespace triagens {
  namespace mvcc {
    class TransactionCollection;
    class TransactionScope;
  }

  namespace arango {

// -----------------------------------------------------------------------------
// --SECTION--                                              RestMvccImportResult
// -----------------------------------------------------------------------------

    struct RestMvccImportResult {

      public:
        RestMvccImportResult () :
          _numErrors(0),
          _numEmpty(0),
          _numCreated(0),
          _errors() {
        }

        ~RestMvccImportResult () { }

        size_t _numErrors;
        size_t _numEmpty;
        size_t _numCreated;

        std::vector<std::string> _errors;
    };

////////////////////////////////////////////////////////////////////////////////
/// @brief import request handler
////////////////////////////////////////////////////////////////////////////////

    class RestMvccImportHandler : public RestVocbaseBaseHandler {

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief constructor
////////////////////////////////////////////////////////////////////////////////

        RestMvccImportHandler (rest::HttpRequest*);

// -----------------------------------------------------------------------------
// --SECTION--                                                   Handler methods
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        status_t execute () override final;

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

    private:

////////////////////////////////////////////////////////////////////////////////
/// @brief create a position string
////////////////////////////////////////////////////////////////////////////////

      std::string positionize (size_t) const;

////////////////////////////////////////////////////////////////////////////////
/// @brief register an error
////////////////////////////////////////////////////////////////////////////////

      void registerError (RestMvccImportResult&,
                          std::string const&);

////////////////////////////////////////////////////////////////////////////////
/// @brief construct an error message
////////////////////////////////////////////////////////////////////////////////

      std::string buildParseError (size_t,
                                   char const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief process a single JSON document
////////////////////////////////////////////////////////////////////////////////

      int handleSingleDocument (triagens::mvcc::TransactionScope*,
                                triagens::mvcc::TransactionCollection*,
                                triagens::mvcc::OperationOptions const&,
                                char const*,
                                TRI_json_t const*,
                                std::string&,
                                size_t);

////////////////////////////////////////////////////////////////////////////////
/// @brief creates documents by JSON objects
/// each line of the input stream contains an individual JSON object
////////////////////////////////////////////////////////////////////////////////

      bool createFromJson (std::string const&);

////////////////////////////////////////////////////////////////////////////////
/// @brief creates documents by JSON objects
/// the input stream is one big JSON array containing all documents
////////////////////////////////////////////////////////////////////////////////

      bool createByDocumentsList ();

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a documents from key/value lists
////////////////////////////////////////////////////////////////////////////////

      bool createFromKeyValueList ();

////////////////////////////////////////////////////////////////////////////////
/// @brief creates the result
////////////////////////////////////////////////////////////////////////////////

      void generateDocumentsCreated (RestMvccImportResult const&);

////////////////////////////////////////////////////////////////////////////////
/// @brief parses a string
////////////////////////////////////////////////////////////////////////////////

      TRI_json_t* parseJsonLine (std::string const&);

////////////////////////////////////////////////////////////////////////////////
/// @brief parses a string
////////////////////////////////////////////////////////////////////////////////

      TRI_json_t* parseJsonLine (char const*,
                                 char const*);

////////////////////////////////////////////////////////////////////////////////
/// @brief builds a TRI_json_t object from a key and value list
////////////////////////////////////////////////////////////////////////////////

      TRI_json_t* createJsonObject (TRI_json_t const*,
                                    TRI_json_t const*,
                                    std::string&,
                                    size_t);

////////////////////////////////////////////////////////////////////////////////
/// @brief checks the keys, returns true if all values in the list are strings.
////////////////////////////////////////////////////////////////////////////////

      bool checkKeys (TRI_json_t const*) const;

////////////////////////////////////////////////////////////////////////////////
/// @brief extract a string attribute from a JSON array
///
/// if the attribute is not there or not a string, this returns 0
////////////////////////////////////////////////////////////////////////////////

        char const* extractJsonStringValue (TRI_json_t const*,
                                            char const*);

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
