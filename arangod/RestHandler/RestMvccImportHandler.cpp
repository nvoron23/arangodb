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

#include "RestMvccImportHandler.h"

#include "Basics/JsonHelper.h"
#include "Basics/StringUtils.h"
#include "Mvcc/TransactionCollection.h"
#include "Mvcc/TransactionScope.h"
#include "Rest/HttpRequest.h"
#include "VocBase/document-collection.h"
#include "VocBase/edge-collection.h"
#include "VocBase/vocbase.h"

using namespace std;
using namespace triagens::basics;
using namespace triagens::rest;
using namespace triagens::arango;

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief constructor
////////////////////////////////////////////////////////////////////////////////

RestMvccImportHandler::RestMvccImportHandler (HttpRequest* request)
  : RestVocbaseBaseHandler(request) {
}

// -----------------------------------------------------------------------------
// --SECTION--                                                   Handler methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

HttpHandler::status_t RestMvccImportHandler::execute () {
  if (ServerState::instance()->isCoordinator()) {
    generateError(HttpResponse::NOT_IMPLEMENTED,
                  TRI_ERROR_CLUSTER_UNSUPPORTED,
                  "'/_api/import' is not yet supported in a cluster");
    return status_t(HANDLER_DONE);
  }

  // extract the sub-request type
  HttpRequest::HttpRequestType type = _request->requestType();

  if (type != HttpRequest::HTTP_REQUEST_POST) {
    generateNotImplemented("ILLEGAL " + IMPORT_PATH);
  }
  else {
    // extract the import type
    bool found;
    std::string const documentType = _request->value("type", found);

    if (found &&
        (documentType == "documents" ||
         documentType == "array" ||
         documentType == "list" ||
         documentType == "auto")) {
      createFromJson(documentType);
    }
    else {
      // CSV
      createFromKeyValueList();
    }
  }

  // this handler is done
  return status_t(HANDLER_DONE);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create a position string
////////////////////////////////////////////////////////////////////////////////

std::string RestMvccImportHandler::positionize (size_t i) const {
  return std::string("at position " + std::to_string(i) + ": ");
}

////////////////////////////////////////////////////////////////////////////////
/// @brief register an error
////////////////////////////////////////////////////////////////////////////////

void RestMvccImportHandler::registerError (RestMvccImportResult& result,
                                           std::string const& errorMsg) {
  ++result._numErrors;
  result._errors.push_back(errorMsg);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief construct an error message
////////////////////////////////////////////////////////////////////////////////

std::string RestMvccImportHandler::buildParseError (size_t i,
                                                    char const* lineStart) {
  if (lineStart != nullptr) {
    string part(lineStart);
    if (part.size() > 255) {
      // UTF-8 chars in string will be escaped so we can truncate it at any point
      part = part.substr(0, 255) + "...";
    }
    
    return positionize(i) + "invalid JSON type (expecting object, probably parse error), offending context: " + part;
  }
    
  return positionize(i) + "invalid JSON type (expecting object, probably parse error)";
}

////////////////////////////////////////////////////////////////////////////////
/// @brief process a single JSON document
////////////////////////////////////////////////////////////////////////////////

int RestMvccImportHandler::handleSingleDocument (triagens::mvcc::TransactionScope* transactionScope,
                                                 triagens::mvcc::TransactionCollection* transactionCollection,
                                                 triagens::mvcc::OperationOptions const& options,
                                                 char const* lineStart,
                                                 TRI_json_t const* json,
                                                 std::string& errorMsg,
                                                 size_t i) {
  if (! TRI_IsObjectJson(json)) {
    if (json != nullptr) {
      std::string part = JsonHelper::toString(json);

      if (part.size() > 255) {
        // UTF-8 chars in string will be escaped so we can truncate it at any point
        part = part.substr(0, 255) + "...";
      }
    
      errorMsg = positionize(i) + "invalid JSON type (expecting object), offending document: " + part;
    }
    else {
      errorMsg = buildParseError(i, lineStart);
    }

    return TRI_ERROR_ARANGO_DOCUMENT_TYPE_INVALID;
  }

  // document ok, now import it
  int res = TRI_ERROR_NO_ERROR;

  if (transactionCollection->isEdgeCollection()) {
    char const* from = extractJsonStringValue(json, TRI_VOC_ATTRIBUTE_FROM);
    char const* to   = extractJsonStringValue(json, TRI_VOC_ATTRIBUTE_TO);

    if (from == nullptr || to == nullptr) {
      std::string part = JsonHelper::toString(json);
      if (part.size() > 255) {
        // UTF-8 chars in string will be escaped so we can truncate it at any point
        part = part.substr(0, 255) + "...";
      }
    
      errorMsg = positionize(i) + "missing '_from' or '_to' attribute, offending document: " + part;

      return TRI_ERROR_ARANGO_INVALID_EDGE_ATTRIBUTE;
    }

    TRI_document_edge_t edge = { 0, nullptr, 0, nullptr };
    std::unique_ptr<char[]> fromKey;
    std::unique_ptr<char[]> toKey;

    // Note that in a DBserver in a cluster the following two calls will
    // parse the first part as a cluster-wide collection name:
    auto resolver = transactionScope->transaction()->resolver();
    int res1 = parseDocumentId(resolver, from, edge._fromCid, fromKey);
    int res2 = parseDocumentId(resolver, to, edge._toCid, toKey);
    
    edge._fromKey = fromKey.get();
    edge._toKey   = toKey.get();

    if (res1 == TRI_ERROR_NO_ERROR && res2 == TRI_ERROR_NO_ERROR) {
      auto document = triagens::mvcc::Document::CreateFromJson(transactionCollection->shaper(), json);
      document.edge = &edge; 
      auto insertResult = triagens::mvcc::CollectionOperations::InsertDocument(transactionScope, transactionCollection, document, options);
      res = insertResult.code;
    }
    else {
      res = (res1 != TRI_ERROR_NO_ERROR ? res1 : res2);
    }
  }
  else {
    auto document = triagens::mvcc::Document::CreateFromJson(transactionCollection->shaper(), json);
    auto insertResult = triagens::mvcc::CollectionOperations::InsertDocument(transactionScope, transactionCollection, document, options);
    res = insertResult.code;
  }

  if (res != TRI_ERROR_NO_ERROR) {
    std::string part = JsonHelper::toString(json);
    if (part.size() > 255) {
      // UTF-8 chars in string will be escaped so we can truncate it at any point
      part = part.substr(0, 255) + "...";
    }

    errorMsg = positionize(i) +
               "creating document failed with error '" + TRI_errno_string(res) +
               "', offending document: " + part;
  }

  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief imports documents from JSON
///
/// @RESTHEADER{POST /_api/import,imports documents from JSON}
///
/// @RESTBODYPARAM{documents,string,required}
/// The body must either be a JSON-encoded array of objects or a string with
/// multiple JSON object separated by newlines.
///
/// @RESTQUERYPARAMETERS
///
/// @RESTQUERYPARAM{type,string,required}
/// Determines how the body of the request will be interpreted. `type` can have
/// the following values:
/// - `documents`: when this type is used, each line in the request body is
///   expected to be an individual JSON-encoded document. Multiple JSON objects
///   in the request body need to be separated by newlines.
/// - `list`: when this type is used, the request body must contain a single
///   JSON-encoded array of individual objects to import.
/// - `auto`: if set, this will automatically determine the body type (either
///   `documents` or `list`).
///
/// @RESTQUERYPARAM{collection,string,required}
/// The collection name.
///
/// @RESTQUERYPARAM{createCollection,boolean,optional}
/// If this parameter has a value of `true` or `yes`, then the collection is
/// created if it does not yet exist. Other values will be ignored so the
/// collection must be present for the operation to succeed.
///
/// @RESTQUERYPARAM{overwrite,boolean,optional}
/// If this parameter has a value of `true` or `yes`, then all data in the
/// collection will be removed prior to the import. Note that any existing
/// index definitions will be preseved.
///
/// @RESTQUERYPARAM{waitForSync,boolean,optional}
/// Wait until documents have been synced to disk before returning.
///
/// @RESTQUERYPARAM{complete,boolean,optional}
/// If set to `true` or `yes`, it will make the whole import fail if any error
/// occurs. Otherwise the import will continue even if some documents cannot
/// be imported.
///
/// @RESTQUERYPARAM{details,boolean,optional}
/// If set to `true` or `yes`, the result will include an attribute `details`
/// with details about documents that could not be imported.
///
/// @RESTDESCRIPTION
/// Creates documents in the collection identified by `collection-name`.
/// The JSON representations of the documents must be passed as the body of the
/// POST request. The request body can either consist of multiple lines, with
/// each line being a single stand-alone JSON object, or a singe JSON array with
/// sub-objects.
///
/// The response is a JSON object with the following attributes:
///
/// - `created`: number of documents imported.
///
/// - `errors`: number of documents that were not imported due to an error.
///
/// - `empty`: number of empty lines found in the input (will only contain a
///   value greater zero for types `documents` or `auto`).
///
/// - `details`: if URL parameter `details` is set to true, the result will
///   contain a `details` attribute which is an array with more detailed
///   information about which documents could not be inserted.
///
/// @RESTRETURNCODES
///
/// @RESTRETURNCODE{201}
/// is returned if all documents could be imported successfully.
///
/// @RESTRETURNCODE{400}
/// is returned if `type` contains an invalid value, no `collection` is
/// specified, the documents are incorrectly encoded, or the request
/// is malformed.
///
/// @RESTRETURNCODE{404}
/// is returned if `collection` or the `_from` or `_to` attributes of an
/// imported edge refer to an unknown collection.
///
/// @RESTRETURNCODE{409}
/// is returned if the import would trigger a unique key violation and
/// `complete` is set to `true`.
///
/// @RESTRETURNCODE{500}
/// is returned if the server cannot auto-generate a document key (out of keys
/// error) for a document with no user-defined key.
///
/// @EXAMPLES
///
/// Importing documents with heterogenous attributes from a JSON array:
///
/// @EXAMPLE_ARANGOSH_RUN{RestImportJsonList}
///     db._flushCache();
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///     db._flushCache();
///
///     var body = [
///       { _key: "abc", value1: 25, value2: "test", allowed: true },
///       { _key: "foo", name: "baz" },
///       { name: { detailed: "detailed name", short: "short name" } }
///     ];
///
///     var response = logCurlRequestRaw('POST', "/_api/import?collection=" + cn + "&type=list", JSON.stringify(body));
///
///     assert(response.code === 201);
///     var r = JSON.parse(response.body);
///     assert(r.created === 3);
///     assert(r.errors === 0);
///     assert(r.empty === 0);
///
///     logJsonResponse(response);
///     db._drop(cn);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Importing documents from individual JSON lines:
///
/// @EXAMPLE_ARANGOSH_RUN{RestImportJsonLines}
///     db._flushCache();
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///     db._flushCache();
///
///     var body = '{ "_key": "abc", "value1": 25, "value2": "test", "allowed": true }\n{ "_key": "foo", "name": "baz" }\n\n{ "name": { "detailed": "detailed name", "short": "short name" } }\n';
///     var response = logCurlRequestRaw('POST', "/_api/import?collection=" + cn + "&type=documents", body);
///
///     assert(response.code === 201);
///     var r = JSON.parse(response.body);
///     assert(r.created === 3);
///     assert(r.errors === 0);
///     assert(r.empty === 1);
///
///     logJsonResponse(response);
///     db._drop(cn);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Using the auto type detection:
///
/// @EXAMPLE_ARANGOSH_RUN{RestImportJsonType}
///     db._flushCache();
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///     db._flushCache();
///
///     var body = [
///       { _key: "abc", value1: 25, value2: "test", allowed: true },
///       { _key: "foo", name: "baz" },
///       { name: { detailed: "detailed name", short: "short name" } }
///     ];
///
///     var response = logCurlRequestRaw('POST', "/_api/import?collection=" + cn + "&type=auto", JSON.stringify(body));
///
///     assert(response.code === 201);
///     var r = JSON.parse(response.body);
///     assert(r.created === 3);
///     assert(r.errors === 0);
///     assert(r.empty === 0);
///
///     logJsonResponse(response);
///     db._drop(cn);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Importing documents into a new collection from a JSON array:
///
/// @EXAMPLE_ARANGOSH_RUN{RestImportJsonCreate}
///     db._flushCache();
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///     db._flushCache();
///
///     var body = [
///       { id: "12553", active: true },
///       { id: "4433", active: false },
///       { id: "55932", count: 4334 },
///     ];
///
///     var response = logCurlRequestRaw('POST', "/_api/import?collection=" + cn + "&createCollection=true&type=list", JSON.stringify(body));
///
///     assert(response.code === 201);
///     var r = JSON.parse(response.body);
///     assert(r.created === 3);
///     assert(r.errors === 0);
///     assert(r.empty === 0);
///
///     logJsonResponse(response);
///     db._drop(cn);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Importing into an edge collection, with attributes `_from`, `_to` and `name`:
///
/// @EXAMPLE_ARANGOSH_RUN{RestImportJsonEdge}
///     db._flushCache();
///     var cn = "links";
///     db._drop(cn);
///     db._createEdgeCollection(cn);
///     db._drop("products");
///     db._create("products");
///     db._flushCache();
///
///     var body = '{ "_from": "products/123", "_to": "products/234" }\n{ "_from": "products/332", "_to": "products/abc", "name": "other name" }';
///
///     var response = logCurlRequestRaw('POST', "/_api/import?collection=" + cn + "&type=documents", body);
///
///     assert(response.code === 201);
///     var r = JSON.parse(response.body);
///     assert(r.created === 2);
///     assert(r.errors === 0);
///     assert(r.empty === 0);
///
///     logJsonResponse(response);
///     db._drop(cn);
///     db._drop("products");
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Importing into an edge collection, omitting `_from` or `_to`:
///
/// @EXAMPLE_ARANGOSH_RUN{RestImportJsonEdgeInvalid}
///     db._flushCache();
///     var cn = "links";
///     db._drop(cn);
///     db._createEdgeCollection(cn);
///     db._flushCache();
///
///     var body = [ { name: "some name" } ];
///
///     var response = logCurlRequestRaw('POST', "/_api/import?collection=" + cn + "&type=list&details=true", JSON.stringify(body));
///
///     assert(response.code === 201);
///     var r = JSON.parse(response.body);
///     assert(r.created === 0);
///     assert(r.errors === 1);
///     assert(r.empty === 0);
///
///     logJsonResponse(response);
///     db._drop(cn);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Violating a unique constraint, but allow partial imports:
///
/// @EXAMPLE_ARANGOSH_RUN{RestImportJsonUniqueContinue}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///     db._flushCache();
///
///     var body = '{ "_key": "abc", "value1": 25, "value2": "test" }\n{ "_key": "abc", "value1": "bar", "value2": "baz" }';
///
///     var response = logCurlRequestRaw('POST', "/_api/import?collection=" + cn + "&type=documents&details=true", body);
///
///     assert(response.code === 201);
///     var r = JSON.parse(response.body);
///     assert(r.created === 1);
///     assert(r.errors === 1);
///     assert(r.empty === 0);
///
///     logJsonResponse(response);
///     db._drop(cn);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Violating a unique constraint, not allowing partial imports:
///
/// @EXAMPLE_ARANGOSH_RUN{RestImportJsonUniqueFail}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///     db._flushCache();
///
///     var body = '{ "_key": "abc", "value1": 25, "value2": "test" }\n{ "_key": "abc", "value1": "bar", "value2": "baz" }';
///
///     var response = logCurlRequestRaw('POST', "/_api/import?collection=" + cn + "&type=documents&complete=true", body);
///
///     assert(response.code === 409);
///
///     logJsonResponse(response);
///     db._drop(cn);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Using a non-existing collection:
///
/// @EXAMPLE_ARANGOSH_RUN{RestImportJsonInvalidCollection}
///     var cn = "products";
///     db._drop(cn);
///
///     var body = '{ "name": "test" }';
///
///     var response = logCurlRequestRaw('POST', "/_api/import?collection=" + cn + "&type=documents", body);
///
///     assert(response.code === 404);
///
///     logJsonResponse(response);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Using a malformed body:
///
/// @EXAMPLE_ARANGOSH_RUN{RestImportJsonInvalidBody}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///     db._flushCache();
///
///     var body = '{ }';
///
///     var response = logCurlRequestRaw('POST', "/_api/import?collection=" + cn + "&type=list", body);
///
///     assert(response.code === 400);
///
///     logJsonResponse(response);
///     db._drop(cn);
/// @END_EXAMPLE_ARANGOSH_RUN
////////////////////////////////////////////////////////////////////////////////

bool RestMvccImportHandler::createFromJson (std::string const& type) {
  RestMvccImportResult result;

  vector<string> const& suffix = _request->suffix();

  if (! suffix.empty()) {
    generateError(HttpResponse::BAD,
                  TRI_ERROR_HTTP_SUPERFLUOUS_SUFFICES,
                  "superfluous suffix, expecting " + IMPORT_PATH + "?collection=<identifier>");
    return false;
  }

  // extract the collection name
  bool found;
  string const& collectionName = _request->value("collection", found);

  if (! found || collectionName.empty()) {
    generateError(HttpResponse::BAD,
                  TRI_ERROR_ARANGO_COLLECTION_PARAMETER_MISSING,
                  "'collection' is missing, expecting " + IMPORT_PATH + "?collection=<identifier>");
    return false;
  }

  if (! checkCreateCollection(collectionName, TRI_COL_TYPE_DOCUMENT)) {
    return false;
  }

  bool linewise;

  if (type == "documents") {
    // linewise import
    linewise = true;
  }
  else if (type == "array" || type == "list") {
    // non-linewise import
    linewise = false;
  }
  else if (type == "auto") {
    linewise = true;

    // auto detect import type by peeking at first character
    char const* ptr = _request->body();
    char const* end = ptr + _request->bodySize();

    while (ptr < end) {
      char const c = *ptr;
      if (c == '\r' || c == '\n' || c == '\t' || c == '\b' || c == '\f' || c == ' ') {
        ptr++;
        continue;
      }
      else if (c == '[') {
        linewise = false;
      }

      break;
    }
  }
  else {
    generateError(HttpResponse::BAD,
                  TRI_ERROR_BAD_PARAMETER,
                  "invalid value for 'type'");
    return false;
  }
  
  // need a fake old transaction in order to not throw - can be removed later       
  TransactionBase oldTrx(true);
    
  try {
    // force a sub-transaction so we can rollback the entire truncate operation easily
    triagens::mvcc::TransactionScope transactionScope(_vocbase, triagens::mvcc::TransactionScope::NoCollections(), true, true);

    auto* transaction = transactionScope.transaction();
    auto* transactionCollection = transaction->collection(collectionName);
  
    triagens::mvcc::OperationOptions options;
  
    if (extractBoolValue("overwrite", false)) {
      // truncate collection first
      auto truncateResult = triagens::mvcc::CollectionOperations::Truncate(&transactionScope, transactionCollection, options);
    
      if (truncateResult.code != TRI_ERROR_NO_ERROR) {
        THROW_ARANGO_EXCEPTION(truncateResult.code);
      }
    }
    
    options.waitForSync = extractBoolValue("waitForSync", false);
    bool const complete = extractBoolValue("complete", false);

    if (linewise) {
      // each line is a separate JSON document
      char const* ptr = _request->body();
      char const* end = ptr + _request->bodySize();
      size_t i = 0;
      std::string errorMsg;

      while (ptr < end) {
        // read line until done
        i++;
          
        TRI_ASSERT(ptr != nullptr);

        // trim whitespace at start of line
        while (ptr < end && 
              (*ptr == ' ' || *ptr == '\t' || *ptr == '\r' || *ptr == '\b' || *ptr == '\f')) {
          ++ptr;
        }

        if (ptr == end || *ptr == '\0') {
          break;
        }

        // now find end of line
        char const* pos = strchr(ptr, '\n');
        char const* oldPtr = nullptr;

        if (pos == ptr) {
          // line starting with \n, i.e. empty line
          ptr = pos + 1;
          ++result._numEmpty;
          continue;
        }
        
        std::unique_ptr<TRI_json_t> json;

        if (pos != nullptr) {
          // non-empty line
          *(const_cast<char*>(pos)) = '\0';
          TRI_ASSERT(ptr != nullptr);
          oldPtr = ptr;
          json.reset(parseJsonLine(ptr, pos));
          ptr = pos + 1;
        }
        else {
          // last-line, non-empty
          TRI_ASSERT(pos == nullptr);
          TRI_ASSERT(ptr != nullptr);
          oldPtr = ptr;
          json.reset(parseJsonLine(ptr));
          ptr = end;
        }

        int res = handleSingleDocument(&transactionScope, transactionCollection, options, oldPtr, json.get(), errorMsg, i);

        if (res == TRI_ERROR_NO_ERROR) {
          ++result._numCreated;
        }
        else {
          registerError(result, errorMsg);

          if (complete) {
            // only perform a full import: abort
            THROW_ARANGO_EXCEPTION(res);
          }
        }
      }
    }

    else {
      // the entire request body is one JSON document
      std::unique_ptr<TRI_json_t> documents(TRI_Json2String(TRI_UNKNOWN_MEM_ZONE, _request->body(), nullptr));

      if (! TRI_IsArrayJson(documents.get())) {
        generateError(HttpResponse::BAD,
                      TRI_ERROR_HTTP_BAD_PARAMETER,
                      "expecting a JSON array in the request");
        return false;
      }

      std::string errorMsg;
      auto const* json = documents.get(); 
      size_t const n = TRI_LengthArrayJson(json);

      for (size_t i = 0; i < n; ++i) {
        auto document = static_cast<TRI_json_t const*>(TRI_AtVector(&json->_value._objects, i));

        int res = handleSingleDocument(&transactionScope, transactionCollection, options, nullptr, document, errorMsg, i + 1);

        if (res == TRI_ERROR_NO_ERROR) {
          ++result._numCreated;
        }
        else {
          registerError(result, errorMsg);

          if (complete) {
            // only perform a full import: abort
            THROW_ARANGO_EXCEPTION(res);
          }
        }
      }
    }

    transactionScope.commit();
    // generate result
    generateDocumentsCreated(result);
    return true;
  }
  catch (triagens::arango::Exception const& ex) {
    generateError(HttpResponse::responseCode(ex.code()), ex.code(), ex.what());
    return false;
  }
  catch (...) {
    generateError(HttpResponse::SERVER_ERROR, TRI_ERROR_INTERNAL);
    return false;
  }

  // unreachable
  TRI_ASSERT(false); 
}

////////////////////////////////////////////////////////////////////////////////
/// @brief imports documents from JSON-encoded lists
///
/// @RESTHEADER{POST /_api/import,imports document values}
///
/// @RESTBODYPARAM{documents,string,required}
/// The body must consist of JSON-encoded arrays of attribute values, with one
/// line per document. The first row of the request must be a JSON-encoded
/// array of attribute names. These attribute names are used for the data in the
/// subsequent lines.
///
/// @RESTQUERYPARAMETERS
///
/// @RESTQUERYPARAM{collection,string,required}
/// The collection name.
///
/// @RESTQUERYPARAM{createCollection,boolean,optional}
/// If this parameter has a value of `true` or `yes`, then the collection is
/// created if it does not yet exist. Other values will be ignored so the
/// collection must be present for the operation to succeed.
///
/// @RESTQUERYPARAM{overwrite,boolean,optional}
/// If this parameter has a value of `true` or `yes`, then all data in the
/// collection will be removed prior to the import. Note that any existing
/// index definitions will be preseved.
///
/// @RESTQUERYPARAM{waitForSync,boolean,optional}
/// Wait until documents have been synced to disk before returning.
///
/// @RESTQUERYPARAM{complete,boolean,optional}
/// If set to `true` or `yes`, it will make the whole import fail if any error
/// occurs. Otherwise the import will continue even if some documents cannot
/// be imported.
///
/// @RESTQUERYPARAM{details,boolean,optional}
/// If set to `true` or `yes`, the result will include an attribute `details`
/// with details about documents that could not be imported.
///
/// @RESTDESCRIPTION
/// Creates documents in the collection identified by `collection-name`.
/// The first line of the request body must contain a JSON-encoded array of
/// attribute names. All following lines in the request body must contain
/// JSON-encoded arrays of attribute values. Each line is interpreted as a
/// separate document, and the values specified will be mapped to the array
/// of attribute names specified in the first header line.
///
/// The response is a JSON object with the following attributes:
///
/// - `created`: number of documents imported.
///
/// - `errors`: number of documents that were not imported due to an error.
///
/// - `empty`: number of empty lines found in the input (will only contain a
///   value greater zero for types `documents` or `auto`).
///
/// - `details`: if URL parameter `details` is set to true, the result will
///   contain a `details` attribute which is an array with more detailed
///   information about which documents could not be inserted.
///
/// @RESTRETURNCODES
///
/// @RESTRETURNCODE{201}
/// is returned if all documents could be imported successfully.
///
/// @RESTRETURNCODE{400}
/// is returned if `type` contains an invalid value, no `collection` is
/// specified, the documents are incorrectly encoded, or the request
/// is malformed.
///
/// @RESTRETURNCODE{404}
/// is returned if `collection` or the `_from` or `_to` attributes of an
/// imported edge refer to an unknown collection.
///
/// @RESTRETURNCODE{409}
/// is returned if the import would trigger a unique key violation and
/// `complete` is set to `true`.
///
/// @RESTRETURNCODE{500}
/// is returned if the server cannot auto-generate a document key (out of keys
/// error) for a document with no user-defined key.
///
/// @EXAMPLES
///
/// Importing two documents, with attributes `_key`, `value1` and `value2` each. One
/// line in the import data is empty:
///
/// @EXAMPLE_ARANGOSH_RUN{RestImportCsvExample}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///
///     var body = '[ "_key", "value1", "value2" ]\n[ "abc", 25, "test" ]\n\n[ "foo", "bar", "baz" ]';
///
///     var response = logCurlRequestRaw('POST', "/_api/import?collection=" + cn, body);
///
///     assert(response.code === 201);
///     var r = JSON.parse(response.body)
///     assert(r.created === 2);
///     assert(r.errors === 0);
///     assert(r.empty === 1);
///
///     logJsonResponse(response);
///     db._drop(cn);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Importing two documents into a new collection:
///
/// @EXAMPLE_ARANGOSH_RUN{RestImportCsvCreate}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///
///     var body = '[ "value1", "value2" ]\n[ 1234, null ]\n[ "foo", "bar" ]\n[ 534.55, true ]';
///
///     var response = logCurlRequestRaw('POST', "/_api/import?collection=" + cn + "&createCollection=true", body);
///
///     assert(response.code === 201);
///     var r = JSON.parse(response.body)
///     assert(r.created === 3);
///     assert(r.errors === 0);
///     assert(r.empty === 0);
///
///     logJsonResponse(response);
///     db._drop(cn);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Importing into an edge collection, with attributes `_from`, `_to` and `name`:
///
/// @EXAMPLE_ARANGOSH_RUN{RestImportCsvEdge}
///     var cn = "links";
///     db._drop(cn);
///     db._createEdgeCollection(cn);
///     db._drop("products");
///     db._create("products");
///
///     var body = '[ "_from", "_to", "name" ]\n[ "products/123", "products/234", "some name" ]\n[ "products/332", "products/abc", "other name" ]';
///
///     var response = logCurlRequestRaw('POST', "/_api/import?collection=" + cn, body);
///
///     assert(response.code === 201);
///     var r = JSON.parse(response.body)
///     assert(r.created === 2);
///     assert(r.errors === 0);
///     assert(r.empty === 0);
///
///     logJsonResponse(response);
///     db._drop(cn);
///     db._drop("products");
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Importing into an edge collection, omitting `_from` or `_to`:
///
/// @EXAMPLE_ARANGOSH_RUN{RestImportCsvEdgeInvalid}
///     var cn = "links";
///     db._drop(cn);
///     db._createEdgeCollection(cn);
///
///     var body = '[ "name" ]\n[ "some name" ]\n[ "other name" ]';
///
///     var response = logCurlRequestRaw('POST', "/_api/import?collection=" + cn + "&details=true", body);
///
///     assert(response.code === 201);
///     var r = JSON.parse(response.body)
///     assert(r.created === 0);
///     assert(r.errors === 2);
///     assert(r.empty === 0);
///
///     logJsonResponse(response);
///     db._drop(cn);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Violating a unique constraint, but allow partial imports:
///
/// @EXAMPLE_ARANGOSH_RUN{RestImportCsvUniqueContinue}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///
///     var body = '[ "_key", "value1", "value2" ]\n[ "abc", 25, "test" ]\n[ "abc", "bar", "baz" ]';
///
///     var response = logCurlRequestRaw('POST', "/_api/import?collection=" + cn + "&details=true", body);
///
///     assert(response.code === 201);
///     var r = JSON.parse(response.body)
///     assert(r.created === 1);
///     assert(r.errors === 1);
///     assert(r.empty === 0);
///
///     logJsonResponse(response);
///     db._drop(cn);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Violating a unique constraint, not allowing partial imports:
///
/// @EXAMPLE_ARANGOSH_RUN{RestImportCsvUniqueFail}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///
///     var body = '[ "_key", "value1", "value2" ]\n[ "abc", 25, "test" ]\n[ "abc", "bar", "baz" ]';
///
///     var response = logCurlRequest('POST', "/_api/import?collection=" + cn + "&complete=true", body);
///
///     assert(response.code === 409);
///
///     logJsonResponse(response);
///     db._drop(cn);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Using a non-existing collection:
///
/// @EXAMPLE_ARANGOSH_RUN{RestImportCsvInvalidCollection}
///     var cn = "products";
///     db._drop(cn);
///
///     var body = '[ "_key", "value1", "value2" ]\n[ "abc", 25, "test" ]\n[ "foo", "bar", "baz" ]';
///
///     var response = logCurlRequest('POST', "/_api/import?collection=" + cn, body);
///
///     assert(response.code === 404);
///
///     logJsonResponse(response);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Using a malformed body:
///
/// @EXAMPLE_ARANGOSH_RUN{RestImportCsvInvalidBody}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///
///     var body = '{ "_key": "foo", "value1": "bar" }';
///
///     var response = logCurlRequest('POST', "/_api/import?collection=" + cn, body);
///
///     assert(response.code === 400);
///
///     logJsonResponse(response);
///     db._drop(cn);
/// @END_EXAMPLE_ARANGOSH_RUN
////////////////////////////////////////////////////////////////////////////////

bool RestMvccImportHandler::createFromKeyValueList () {
  RestMvccImportResult result;

  vector<string> const& suffix = _request->suffix();

  if (! suffix.empty()) {
    generateError(HttpResponse::BAD,
                  TRI_ERROR_HTTP_SUPERFLUOUS_SUFFICES,
                  "superfluous suffix, expecting " + IMPORT_PATH + "?collection=<identifier>");
    return false;
  }

  // extract the collection name
  bool found;
  string const& collectionName = _request->value("collection", found);

  if (! found || collectionName.empty()) {
    generateError(HttpResponse::BAD,
                  TRI_ERROR_ARANGO_COLLECTION_PARAMETER_MISSING,
                  "'collection' is missing, expecting " + IMPORT_PATH + "?collection=<identifier>");
    return false;
  }

  if (! checkCreateCollection(collectionName, TRI_COL_TYPE_DOCUMENT)) {
    return false;
  }

  // read line number (optional)
  int64_t lineNumber = 0;
  std::string const& lineNumValue = _request->value("line", found);
  if (found) {
    lineNumber = StringUtils::int64(lineNumValue);
  }

  char const* current = _request->body();
  char const* bodyEnd = current + _request->bodySize();

  // process header
  char const* next = strchr(current, '\n');

  if (next == nullptr) {
    generateError(HttpResponse::BAD,
                  TRI_ERROR_HTTP_BAD_PARAMETER,
                  "no JSON array found in second line");
    return false;
  }
  
  char const* lineStart = current;
  char const* lineEnd   = next;
    
  // trim line
  while (lineStart < bodyEnd &&
         (*lineStart == ' ' || *lineStart == '\t' || *lineStart == '\r' || *lineStart == '\n' || *lineStart == '\b' || *lineStart == '\f')) {
    ++lineStart;
  }
    
  while (lineEnd > lineStart &&
         (*(lineEnd - 1) == ' ' || *(lineEnd - 1) == '\t' || *(lineEnd - 1) == '\r' || *(lineEnd - 1) == '\n' || *(lineEnd - 1) == '\b' || *(lineEnd - 1) == '\f')) {
    --lineEnd;
  }

  *(const_cast<char*>(lineEnd)) = '\0';

  // read first line with attribute names
  std::unique_ptr<TRI_json_t> keys(parseJsonLine(lineStart, lineEnd));

  if (! checkKeys(keys.get())) {
    generateError(HttpResponse::BAD,
                  TRI_ERROR_HTTP_BAD_PARAMETER,
                  "no JSON string array found in first line");
    return false;
  }

  current = next + 1;
  
  // need a fake old transaction in order to not throw - can be removed later       
  TransactionBase oldTrx(true);
    
  try {
    // force a sub-transaction so we can rollback the entire truncate operation easily
    triagens::mvcc::TransactionScope transactionScope(_vocbase, triagens::mvcc::TransactionScope::NoCollections(), true, true);

    auto* transaction = transactionScope.transaction();
    auto* transactionCollection = transaction->collection(collectionName);
    
    triagens::mvcc::OperationOptions options;
  
    if (extractBoolValue("overwrite", false)) {
      // truncate collection first
      auto truncateResult = triagens::mvcc::CollectionOperations::Truncate(&transactionScope, transactionCollection, options);
    
      if (truncateResult.code != TRI_ERROR_NO_ERROR) {
        THROW_ARANGO_EXCEPTION(truncateResult.code);
      }
    }
 
    options.waitForSync = extractBoolValue("waitForSync", false);
    bool const complete    = extractBoolValue("complete", false);

    size_t i = static_cast<size_t>(lineNumber);

    while (current != nullptr && current < bodyEnd) {
      i++;

      next = static_cast<char const*>(memchr(current, '\n', bodyEnd - current));

      char const* lineStart = current;
      char const* lineEnd   = next;

      if (next == nullptr) {
        // reached the end
        lineEnd = bodyEnd;
        current = nullptr;
      }
      else {
        // got more to read
        current = next + 1;
        *(const_cast<char*>(lineEnd)) = '\0';
      }

      // trim line
      while (lineStart < bodyEnd &&
            (*lineStart == ' ' || *lineStart == '\t' || *lineStart == '\r' || *lineStart == '\n' || *lineStart == '\b' || *lineStart == '\f')) {
        ++lineStart;
      }
      
      while (lineEnd > lineStart &&
            (*(lineEnd - 1) == ' ' || *(lineEnd - 1) == '\t' || *(lineEnd - 1) == '\r' || *(lineEnd - 1) == '\n' || *(lineEnd - 1) == '\b' || *(lineEnd - 1) == '\f')) {
        --lineEnd;
      }

      if (lineStart == lineEnd) {
        ++result._numEmpty;
        continue;
      }

      std::unique_ptr<TRI_json_t> values(parseJsonLine(lineStart, lineEnd));

      if (values.get() != nullptr) {
        // build the json object from the array
        std::string errorMsg;

        std::unique_ptr<TRI_json_t> json(createJsonObject(keys.get(), values.get(), errorMsg, i));

        int res;
        if (json.get() != nullptr) {
          res = handleSingleDocument(&transactionScope, transactionCollection, options, lineStart, json.get(), errorMsg, i);
        }
        else {
          // raise any error
          res = TRI_ERROR_INTERNAL;
        }

        if (res == TRI_ERROR_NO_ERROR) {
          ++result._numCreated;
        }
        else {
          registerError(result, errorMsg);

          if (complete) {
            // only perform a full import: abort
            THROW_ARANGO_EXCEPTION(res);
          }
        }
      }
      else {
        string errorMsg = buildParseError(i, lineStart);
        registerError(result, errorMsg);
        // TODO: throw here?
      }
    }

    transactionScope.commit();
    // generate result
    generateDocumentsCreated(result);
    return true;
  }
  catch (triagens::arango::Exception const& ex) {
    generateError(HttpResponse::responseCode(ex.code()), ex.code(), ex.what());
    return false;
  }
  catch (...) {
    generateError(HttpResponse::SERVER_ERROR, TRI_ERROR_INTERNAL);
    return false;
  }

  // unreachable
  TRI_ASSERT(false); 
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create response for number of documents created / failed
////////////////////////////////////////////////////////////////////////////////

void RestMvccImportHandler::generateDocumentsCreated (RestMvccImportResult const& result) {
  _response = createResponse(HttpResponse::CREATED);
  _response->setContentType("application/json; charset=utf-8");

  triagens::basics::Json json(triagens::basics::Json::Object, 5);
  json("error", triagens::basics::Json(false));
  json("created", triagens::basics::Json(static_cast<double>(result._numCreated)));
  json("errors", triagens::basics::Json(static_cast<double>(result._numErrors)));
  json("empty", triagens::basics::Json(static_cast<double>(result._numEmpty)));

  bool found;
  char const* detailsStr = _request->value("details", found);

  // include failure details?
  if (found && StringUtils::boolean(detailsStr)) {
    size_t const n = result._errors.size(); 
    triagens::basics::Json messages(triagens::basics::Json::Array, n);

    for (size_t i = 0; i < n; ++i) {
      messages.add(triagens::basics::Json(result._errors[i]));
    }

    json("details", messages);
  }

  generateResult(HttpResponse::CREATED, json.json());
}

////////////////////////////////////////////////////////////////////////////////
/// @brief parse a single document line
////////////////////////////////////////////////////////////////////////////////

TRI_json_t* RestMvccImportHandler::parseJsonLine (std::string const& line) {
  return parseJsonLine(line.c_str(), line.c_str() + line.size());
}

////////////////////////////////////////////////////////////////////////////////
/// @brief parse a single document line
////////////////////////////////////////////////////////////////////////////////

TRI_json_t* RestMvccImportHandler::parseJsonLine (char const* start,
                                                  char const* end) {
  return TRI_Json2String(TRI_UNKNOWN_MEM_ZONE, start, nullptr);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a JSON object from a line containing a document
////////////////////////////////////////////////////////////////////////////////

TRI_json_t* RestMvccImportHandler::createJsonObject (TRI_json_t const* keys,
                                                     TRI_json_t const* values,
                                                     std::string& errorMsg,
                                                     size_t lineNumber) {

  if (! TRI_IsArrayJson(values)) {
    errorMsg = positionize(lineNumber) + "no valid JSON array data";
    return nullptr;
  }

  size_t const n = TRI_LengthArrayJson(keys);

  if (n != TRI_LengthArrayJson(values)) {
    errorMsg = positionize(lineNumber) + "wrong number of JSON values";
    return nullptr;
  }

  TRI_json_t* result = TRI_CreateObjectJson(TRI_UNKNOWN_MEM_ZONE, n);

  if (result == nullptr) {
    return nullptr;
  }

  for (size_t i = 0;  i < n;  ++i) {
    auto key   = static_cast<TRI_json_t const*>(TRI_AtVector(&keys->_value._objects, i));
    auto value = static_cast<TRI_json_t const*>(TRI_AtVector(&values->_value._objects, i));

    if (TRI_IsStringJson(key) && value != nullptr && value->_type > TRI_JSON_NULL) {
      TRI_InsertObjectJson(TRI_UNKNOWN_MEM_ZONE, result, key->_value._string.data, value);
    }
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief validate keys
////////////////////////////////////////////////////////////////////////////////

bool RestMvccImportHandler::checkKeys (TRI_json_t const* keys) const {
  if (! TRI_IsArrayJson(keys)) {
    return false;
  }

  size_t const n = TRI_LengthArrayJson(keys);

  if (n == 0) {
    return false;
  }

  for (size_t i = 0;  i < n;  ++i) {
    auto key = static_cast<TRI_json_t* const>(TRI_AtVector(&keys->_value._objects, i));

    if (! TRI_IsStringJson(key)) {
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief extract a string attribute from a JSON array
///
/// if the attribute is not there or not a string, this returns 0
////////////////////////////////////////////////////////////////////////////////

char const* RestMvccImportHandler::extractJsonStringValue (TRI_json_t const* json,
                                                           char const* name) {
  if (! TRI_IsObjectJson(json)) {
    return nullptr;
  }

  auto value = TRI_LookupObjectJson(json, name);

  if (! TRI_IsStringJson(value)) {
    return nullptr;
  }

  return value->_value._string.data;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
