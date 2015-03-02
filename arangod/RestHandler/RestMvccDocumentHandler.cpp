////////////////////////////////////////////////////////////////////////////////
/// @brief REST mvcc document request handler
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

#include "RestMvccDocumentHandler.h"

#include "Basics/StringUtils.h"
#include "Basics/json.h"
#include "Basics/json-utilities.h"
#include "Cluster/ClusterInfo.h"
#include "Cluster/ClusterComm.h"
#include "Cluster/ClusterMethods.h"
#include "Cluster/ServerState.h"
#include "Mvcc/CollectionOperations.h"
#include "Mvcc/Transaction.h"
#include "Mvcc/TransactionCollection.h"
#include "Mvcc/TransactionScope.h"
#include "Rest/HttpRequest.h"
#include "VocBase/document-collection.h"
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

RestMvccDocumentHandler::RestMvccDocumentHandler (HttpRequest* request)
  : RestVocbaseBaseHandler(request) {
}

// -----------------------------------------------------------------------------
// --SECTION--                                                   Handler methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

HttpHandler::status_t RestMvccDocumentHandler::execute () {
  // extract the sub-request type
  HttpRequest::HttpRequestType type = _request->requestType();

  // execute one of the CRUD methods
  switch (type) {
    case HttpRequest::HTTP_REQUEST_DELETE: removeDocument();  break;
    case HttpRequest::HTTP_REQUEST_GET:    readDocument();    break;
    case HttpRequest::HTTP_REQUEST_HEAD:   checkDocument();   break;
    case HttpRequest::HTTP_REQUEST_POST:   insertDocument();  break;
    case HttpRequest::HTTP_REQUEST_PUT:    replaceDocument(); break;
    case HttpRequest::HTTP_REQUEST_PATCH:  updateDocument();  break;

    case HttpRequest::HTTP_REQUEST_ILLEGAL:
    default: {
      generateNotImplemented("ILLEGAL " + DOCUMENT_PATH);
      break;
    }
  }

  // this handler is done
  return status_t(HANDLER_DONE);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                 protected methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock REST_DOCUMENT_CREATE
/// @brief creates a document
///
/// @RESTHEADER{POST /_api/document,Create document}
///
/// @RESTBODYPARAM{document,json,required}
/// A JSON representation of the document.
///
/// @RESTQUERYPARAMETERS
///
/// @RESTQUERYPARAM{collection,string,required}
/// The collection name.
///
/// @RESTQUERYPARAM{createCollection,boolean,optional}
/// If this parameter has a value of *true* or *yes*, then the collection is
/// created if it does not yet exist. Other values will be ignored so the
/// collection must be present for the operation to succeed.
///
/// **Note**: this flag is not supported in a cluster. Using it will result in an
/// error.
///
/// @RESTQUERYPARAM{waitForSync,boolean,optional}
/// Wait until document has been synced to disk.
///
/// @RESTDESCRIPTION
/// Creates a new document in the collection named *collection*.  A JSON
/// representation of the document must be passed as the body of the POST
/// request.
///
/// If the document was created successfully, then the "Location" header
/// contains the path to the newly created document. The "ETag" header field
/// contains the revision of the document.
///
/// The body of the response contains a JSON object with the following
/// attributes:
///
/// - *_id* contains the document handle of the newly created document
/// - *_key* contains the document key
/// - *_rev* contains the document revision
///
/// If the collection parameter *waitForSync* is *false*, then the call returns
/// as soon as the document has been accepted. It will not wait until the
/// document has been synced to disk.
///
/// Optionally, the URL parameter *waitForSync* can be used to force
/// synchronisation of the document creation operation to disk even in case that
/// the *waitForSync* flag had been disabled for the entire collection.  Thus,
/// the *waitForSync* URL parameter can be used to force synchronisation of just
/// this specific operations. To use this, set the *waitForSync* parameter to
/// *true*. If the *waitForSync* parameter is not specified or set to *false*,
/// then the collection's default *waitForSync* behavior is applied. The
/// *waitForSync* URL parameter cannot be used to disable synchronisation for
/// collections that have a default *waitForSync* value of *true*.
///
/// @RESTRETURNCODES
///
/// @RESTRETURNCODE{201}
/// is returned if the document was created successfully and *waitForSync* was
/// *true*.
///
/// @RESTRETURNCODE{202}
/// is returned if the document was created successfully and *waitForSync* was
/// *false*.
///
/// @RESTRETURNCODE{400}
/// is returned if the body does not contain a valid JSON representation of a
/// document. The response body contains an error document in this case.
///
/// @RESTRETURNCODE{404}
/// is returned if the collection specified by *collection* is unknown.  The
/// response body contains an error document in this case.
///
/// @EXAMPLES
///
/// Create a document given a collection named *products*. Note that the
/// revision identifier might or might not by equal to the auto-generated
/// key.
///
/// @EXAMPLE_ARANGOSH_RUN{RestMvccDocumentHandlerPostCreate1}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn, { waitForSync: true });
///
///     var url = "/_api/document?collection=" + cn;
///     var body = '{ "Hello": "World" }';
///
///     var response = logCurlRequest('POST', url, body);
///
///     assert(response.code === 201);
///
///     logJsonResponse(response);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Create a document in a collection named *products* with a collection-level
/// *waitForSync* value of *false*.
///
/// @EXAMPLE_ARANGOSH_RUN{RestMvccDocumentHandlerPostAccept1}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn, { waitForSync: false });
///
///     var url = "/_api/document?collection=" + cn;
///     var body = '{ "Hello": "World" }';
///
///     var response = logCurlRequest('POST', url, body);
///
///     assert(response.code === 202);
///
///     logJsonResponse(response);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Create a document in a collection with a collection-level *waitForSync*
/// value of *false*, but using the *waitForSync* URL parameter.
///
/// @EXAMPLE_ARANGOSH_RUN{RestMvccDocumentHandlerPostWait1}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn, { waitForSync: false });
///
///     var url = "/_api/document?collection=" + cn + "&waitForSync=true";
///     var body = '{ "Hello": "World" }';
///
///     var response = logCurlRequest('POST', url, body);
///
///     assert(response.code === 201);
///
///     logJsonResponse(response);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Create a document in a new, named collection
///
/// @EXAMPLE_ARANGOSH_RUN{RestMvccDocumentHandlerPostCreate2}
///     var cn = "products";
///     db._drop(cn);
///
///     var url = "/_api/document?collection=" + cn + "&createCollection=true";
///     var body = '{ "Hello": "World" }';
///
///     var response = logCurlRequest('POST', url, body);
///
///     assert(response.code === 202);
///
///     logJsonResponse(response);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Unknown collection name:
///
/// @EXAMPLE_ARANGOSH_RUN{RestMvccDocumentHandlerPostUnknownCollection1}
///     var cn = "products";
///     db._drop(cn);
///
///     var url = "/_api/document?collection=" + cn;
///     var body = '{ "Hello": "World" }';
///
///     var response = logCurlRequest('POST', url, body);
///
///     assert(response.code === 404);
///
///     logJsonResponse(response);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Illegal document:
///
/// @EXAMPLE_ARANGOSH_RUN{RestMvccDocumentHandlerPostBadJson1}
///     var cn = "products";
///     db._drop(cn);
///
///     var url = "/_api/document?collection=" + cn;
///     var body = '{ 1: "World" }';
///
///     var response = logCurlRequest('POST', url, body);
///
///     assert(response.code === 400);
///
///     logJsonResponse(response);
/// @END_EXAMPLE_ARANGOSH_RUN
/// @endDocuBlock 
////////////////////////////////////////////////////////////////////////////////

bool RestMvccDocumentHandler::insertDocument () {
  vector<string> const& suffix = _request->suffix();

  if (! suffix.empty()) {
    generateError(HttpResponse::BAD,
                  TRI_ERROR_HTTP_SUPERFLUOUS_SUFFICES,
                  "superfluous suffix, expecting " + MVCC_DOCUMENT_PATH + "?collection=<identifier>");
    return false;
  }

  // extract the cid
  bool found;
  char const* collectionName = _request->value("collection", found);

  if (! found || *collectionName == '\0') {
    generateError(HttpResponse::BAD,
                  TRI_ERROR_ARANGO_COLLECTION_PARAMETER_MISSING,
                  "'collection' is missing, expecting " + MVCC_DOCUMENT_PATH + "?collection=<identifier>");
    return false;
  }

  std::unique_ptr<TRI_json_t> json(parseJsonBody());

  if (json.get() == nullptr) {
    // error message already set in parseJsonBody()
    return false;
  }

  if (json.get()->_type != TRI_JSON_OBJECT) {
    generateError(TRI_ERROR_ARANGO_DOCUMENT_TYPE_INVALID);
    return false;
  }

  if (ServerState::instance()->isCoordinator()) {
    // json will be freed inside!
    return insertDocumentCoordinator(collectionName, extractBoolValue("waitForSync", false), json.release());
  }

  if (! checkCreateCollection(collectionName, getCollectionType())) {
    return false;
  }
  
  // need a fake old transaction in order to not throw - can be removed later       
  TransactionBase oldTrx(true);
  
  try {
    triagens::mvcc::OperationOptions options;
    options.waitForSync = extractBoolValue("waitForSync", false);

    triagens::mvcc::TransactionScope transactionScope(_vocbase, triagens::mvcc::TransactionScope::NoCollections());
    auto* transaction = transactionScope.transaction();
    auto* transactionCollection = transaction->collection(collectionName);
    auto* shaper = transactionCollection->shaper();

    if (transactionCollection->type() != TRI_COL_TYPE_DOCUMENT) {
      // check if we are inserting with the DOCUMENT handler into a non-DOCUMENT collection
      THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_COLLECTION_TYPE_INVALID);
    }

    auto document = triagens::mvcc::Document::CreateFromJson(shaper, json.get());
    auto insertResult = triagens::mvcc::CollectionOperations::InsertDocument(&transactionScope, transactionCollection, document, options);
    
    if (insertResult.code != TRI_ERROR_NO_ERROR) {
      THROW_ARANGO_EXCEPTION(insertResult.code);
    }

    generate20x(insertResponseCode(insertResult.waitForSync), transactionCollection, &insertResult);
    return true;
  }
  catch (triagens::arango::Exception const& ex) { 
    generateError(HttpResponse::responseCode(ex.code()), ex.code(), ex.what());
    return false;
  }
  catch (...) {
    generateError(TRI_ERROR_INTERNAL);
    return false;
  }
 
  // unreachable
  TRI_ASSERT(false);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a document, coordinator case in a cluster
////////////////////////////////////////////////////////////////////////////////

bool RestMvccDocumentHandler::insertDocumentCoordinator (char const* collection,
                                                         bool waitForSync,
                                                         TRI_json_t* json) {
  string const& dbname = _request->databaseName();
  string const collname(collection);
  triagens::rest::HttpResponse::HttpResponseCode responseCode;
  map<string, string> headers = triagens::arango::getForwardableRequestHeaders(_request);
  map<string, string> resultHeaders;
  string resultBody;

  int res = triagens::arango::createDocumentOnCoordinator( 
            dbname, collname, waitForSync, json, headers,
            responseCode, resultHeaders, resultBody);

  if (res != TRI_ERROR_NO_ERROR) {
    generateTransactionError(collection, res);
    return false;
  }

  // Essentially return the response we got from the DBserver, be it
  // OK or an error:
  _response = createResponse(responseCode);
  triagens::arango::mergeResponseHeaders(_response, resultHeaders);
  _response->body().appendText(resultBody.c_str(), resultBody.size());
  return responseCode >= triagens::rest::HttpResponse::BAD;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief reads a single or all documents
///
/// Either readSingleDocument or readAllDocuments.
////////////////////////////////////////////////////////////////////////////////

bool RestMvccDocumentHandler::readDocument () {
  size_t const len = _request->suffix().size();

  switch (len) {
    case 0:
      return readAllDocuments();

    case 1:
      generateError(HttpResponse::BAD,
                    TRI_ERROR_ARANGO_DOCUMENT_HANDLE_BAD,
                    "expecting GET /_api/document/<document-handle>");
      return false;

    case 2:
      return readSingleDocument(true);

    default:
      generateError(HttpResponse::BAD,
                    TRI_ERROR_HTTP_SUPERFLUOUS_SUFFICES,
                    "expecting GET /_api/document/<document-handle> or GET /_api/document?collection=<collection-name>");
      return false;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock REST_DOCUMENT_READ
/// @brief reads a single document
///
/// @RESTHEADER{GET /_api/document/{document-handle},Read document}
///
/// @RESTURLPARAMETERS
///
/// @RESTURLPARAM{document-handle,string,required}
/// The handle of the document.
///
/// @RESTHEADERPARAMETERS
///
/// @RESTHEADERPARAM{If-None-Match,string,optional}
/// If the "If-None-Match" header is given, then it must contain exactly one
/// etag. The document is returned, if it has a different revision than the
/// given etag. Otherwise an *HTTP 304* is returned.
///
/// @RESTHEADERPARAM{If-Match,string,optional}
/// If the "If-Match" header is given, then it must contain exactly one
/// etag. The document is returned, if it has the same revision as the
/// given etag. Otherwise a *HTTP 412* is returned. As an alternative
/// you can supply the etag in an attribute *rev* in the URL.
///
/// @RESTDESCRIPTION
/// Returns the document identified by *document-handle*. The returned
/// document contains three special attributes: *_id* containing the document
/// handle, *_key* containing key which uniquely identifies a document
/// in a given collection and *_rev* containing the revision.
///
/// @RESTRETURNCODES
///
/// @RESTRETURNCODE{200}
/// is returned if the document was found
///
/// @RESTRETURNCODE{304}
/// is returned if the "If-None-Match" header is given and the document has
/// the same version
///
/// @RESTRETURNCODE{404}
/// is returned if the document or collection was not found
///
/// @RESTRETURNCODE{412}
/// is returned if a "If-Match" header or *rev* is given and the found
/// document has a different version. The response will also contain the found
/// document's current revision in the *_rev* attribute. Additionally, the
/// attributes *_id* and *_key* will be returned.
///
/// @EXAMPLES
///
/// Use a document handle:
///
/// @EXAMPLE_ARANGOSH_RUN{RestMvccDocumentHandlerReadDocument}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///
///     var document = db.products.save({"hello":"world"});
///     var url = "/_api/document/" + document._id;
///
///     var response = logCurlRequest('GET', url);
///
///     assert(response.code === 200);
///
///     logJsonResponse(response);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Use a document handle and an etag:
///
/// @EXAMPLE_ARANGOSH_RUN{RestMvccDocumentHandlerReadDocumentIfNoneMatch}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///
///     var document = db.products.save({"hello":"world"});
///     var url = "/_api/document/" + document._id;
///     var headers = {"If-None-Match": "\"" + document._rev + "\""};
///
///     var response = logCurlRequest('GET', url, "", headers);
///
///     assert(response.code === 304);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Unknown document handle:
///
/// @EXAMPLE_ARANGOSH_RUN{RestMvccDocumentHandlerReadDocumentUnknownHandle}
///     var url = "/_api/document/products/unknownhandle";
///
///     var response = logCurlRequest('GET', url);
///
///     assert(response.code === 404);
///
///     logJsonResponse(response);
/// @END_EXAMPLE_ARANGOSH_RUN
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

bool RestMvccDocumentHandler::readSingleDocument (bool generateBody) {
  vector<string> const& suffix = _request->suffix();

  // split the document reference
  string const& collectionName = suffix[0];
  string const& key            = suffix[1];

  // check for an etag
  bool isValidRevision;
  TRI_voc_rid_t const ifNoneRid = extractRevision("if-none-match", 0, isValidRevision);

  if (! isValidRevision) {
    generateError(HttpResponse::BAD,
                  TRI_ERROR_HTTP_BAD_PARAMETER,
                  "invalid revision number");
    return false;
  }

  TRI_voc_rid_t const ifRid = extractRevision("if-match", "rev", isValidRevision);

  if (! isValidRevision) {
    generateError(HttpResponse::BAD,
                  TRI_ERROR_HTTP_BAD_PARAMETER,
                  "invalid revision number");
    return false;
  }

  if (ServerState::instance()->isCoordinator()) {
    return getDocumentCoordinator(collectionName, key, generateBody);
  }
  
  // need a fake old transaction in order to not throw - can be removed later       
  TransactionBase oldTrx(true);

  try {
    triagens::mvcc::OperationOptions options;

    triagens::mvcc::TransactionScope transactionScope(_vocbase, triagens::mvcc::TransactionScope::NoCollections());
    auto* transaction = transactionScope.transaction();
    auto* transactionCollection = transaction->collection(collectionName);

    auto document = triagens::mvcc::Document::CreateFromKey(key, 0);
    auto readResult = triagens::mvcc::CollectionOperations::ReadDocument(&transactionScope, transactionCollection, document, options);
 
    if (readResult.code != TRI_ERROR_NO_ERROR) {
      THROW_ARANGO_EXCEPTION(readResult.code);
    }
    
    // check revision 
    TRI_voc_rid_t const rid = readResult.mptr->_rid;

    if (ifNoneRid == 0 || ifNoneRid != rid) {
      if (ifRid == 0 || ifRid == rid) {
        generateDocument(transactionCollection, &readResult, generateBody);
      }
      else {
        generatePreconditionFailed(transactionCollection, &readResult);
      }
    }
    else {
      if (ifRid == 0 || ifRid == rid) {
        generateNotModified(rid);
      }
      else {
        generatePreconditionFailed(transactionCollection, &readResult);
      }
    }
    return true;
  }
  catch (triagens::arango::Exception const& ex) { 
    generateError(HttpResponse::responseCode(ex.code()), ex.code(), ex.what());
    return false;
  }
  catch (...) {
    generateError(TRI_ERROR_INTERNAL);
    return false;
  }
 
  // unreachable
  TRI_ASSERT(false);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief reads a single a document, coordinator case in a cluster
////////////////////////////////////////////////////////////////////////////////

bool RestMvccDocumentHandler::getDocumentCoordinator (string const& collname,
                                                      string const& key,
                                                      bool generateBody) {
  string const& dbname = _request->databaseName();
  triagens::rest::HttpResponse::HttpResponseCode responseCode;
  map<string, string> headers = triagens::arango::getForwardableRequestHeaders(_request);
  map<string, string> resultHeaders;
  string resultBody;

  // TODO: check if this is ok
  TRI_voc_rid_t rev = 0;
  bool found;
  char const* revstr = _request->value("rev", found);
  if (found) {
    rev = StringUtils::uint64(revstr);
  }

  int error = triagens::arango::getDocumentOnCoordinator(
            dbname, collname, key, rev, headers, generateBody,
            responseCode, resultHeaders, resultBody);

  if (error != TRI_ERROR_NO_ERROR) {
    generateTransactionError(collname, error);
    return false;
  }
  // Essentially return the response we got from the DBserver, be it
  // OK or an error:
  _response = createResponse(responseCode);
  triagens::arango::mergeResponseHeaders(_response, resultHeaders);
  if (! generateBody) {
    // a head request...
    _response->headResponse((size_t) StringUtils::uint64(resultHeaders["content-length"]));
  }
  else {
    _response->body().appendText(resultBody.c_str(), resultBody.size());
  }
  return responseCode >= triagens::rest::HttpResponse::BAD;
}

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock REST_DOCUMENT_READ_ALL
/// @brief reads all documents from collection
///
/// @RESTHEADER{GET /_api/document,Read all documents}
///
/// @RESTQUERYPARAMETERS
///
/// @RESTQUERYPARAM{collection,string,required}
/// The name of the collection.
///
/// @RESTQUERYPARAM{type,string,optional}
/// The type of the result. The following values are allowed:
///
/// - *id*: returns an array of document ids (*_id* attributes)
/// - *key*: returns an array of document keys (*_key* attributes)
/// - *path*: returns an array of document URI paths. This is the default.
///
/// @RESTDESCRIPTION
/// Returns an array of all keys, ids, or URI paths for all documents in the 
/// collection identified by *collection*. The type of the result array is
/// determined by the *type* attribute.
///
/// Note that the results have no defined order and thus the order should
/// not be relied on.
///
/// @RESTRETURNCODES
///
/// @RESTRETURNCODE{200}
/// All went good.
///
/// @RESTRETURNCODE{404}
/// The collection does not exist.
///
/// @EXAMPLES
///
/// Returns all document paths
///
/// @EXAMPLE_ARANGOSH_RUN{RestMvccDocumentHandlerReadDocumentAllPath}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///
///     db.products.save({"hello1":"world1"});
///     db.products.save({"hello2":"world1"});
///     db.products.save({"hello3":"world1"});
///     var url = "/_api/document/?collection=" + cn;
///
///     var response = logCurlRequest('GET', url);
///
///     assert(response.code === 200);
///
///     logJsonResponse(response);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Returns all document keys
///
/// @EXAMPLE_ARANGOSH_RUN{RestMvccDocumentHandlerReadDocumentAllKey}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///
///     db.products.save({"hello1":"world1"});
///     db.products.save({"hello2":"world1"});
///     db.products.save({"hello3":"world1"});
///     var url = "/_api/document/?collection=" + cn + "&type=key";
///
///     var response = logCurlRequest('GET', url);
///
///     assert(response.code === 200);
///
///     logJsonResponse(response);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Collection does not exist.
///
/// @EXAMPLE_ARANGOSH_RUN{RestMvccDocumentHandlerReadDocumentAllCollectionDoesNotExist}
///     var cn = "doesnotexist";
///     db._drop(cn);
///     var url = "/_api/document/?collection=" + cn;
///
///     var response = logCurlRequest('GET', url);
///
///     assert(response.code === 404);
///
///     logJsonResponse(response);
/// @END_EXAMPLE_ARANGOSH_RUN
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

bool RestMvccDocumentHandler::readAllDocuments () {
  bool found;
  std::string collectionName = _request->value("collection", found);
  std::string returnType = _request->value("type", found);

  if (returnType.empty()) {
    returnType = "path";
  }

  if (ServerState::instance()->isCoordinator()) {
    return getAllDocumentsCoordinator(collectionName, returnType);
  }

  // need a fake old transaction in order to not throw - can be removed later       
  TransactionBase oldTrx(true);

  try {
    triagens::mvcc::OperationOptions options;

    triagens::mvcc::TransactionScope transactionScope(_vocbase, triagens::mvcc::TransactionScope::NoCollections());
    auto* transaction = transactionScope.transaction();
    auto* transactionCollection = transaction->collection(collectionName);
  
    // determine the prefix for the results
    string prefix("\"");
  
    if (returnType != "key") {
      if (returnType != "id") {
        // default return type: paths to documents
        if (transactionCollection->type() == TRI_COL_TYPE_EDGE) {
          prefix += StringUtils::replace(MVCC_EDGE_PATH, "/", "\\/") + "\\/";
        }
        else {
          prefix += StringUtils::replace(MVCC_DOCUMENT_PATH, "/", "\\/") + "\\/";
        }
      }
      prefix += transactionCollection->name() + "\\/";
    }
  
    std::vector<TRI_doc_mptr_t const*> documents;
    auto readResult = triagens::mvcc::CollectionOperations::ReadAllDocuments(&transactionScope, transactionCollection, documents, options);

    if (readResult.code != TRI_ERROR_NO_ERROR) {
      generateError(readResult.code);
      return false;
    }
  
    string result;

    // reserve some space in the result: (20 is the header size, 10 is the expected average key size)
    result.reserve(20 + (prefix.size() + 1 + 10) * documents.size());
    result.append("{\"documents\":[");

    bool first = true;
    for (auto const& document : documents) {
      // collection names do not need to be JSON-escaped
      // keys do not need to be JSON-escaped
      result += prefix + TRI_EXTRACT_MARKER_KEY(document) + '"';

      if (first) {
        prefix = "," + prefix;
        first = false;
      }
    }

    result += "]}";

    // and generate a response
    _response = createResponse(HttpResponse::OK);
    _response->setContentType("application/json; charset=utf-8");
    _response->body().appendText(result);

    return true;
  }
  catch (triagens::arango::Exception const& ex) {
    generateError(HttpResponse::responseCode(ex.code()), ex.code(), ex.what());
    return false;
  }
  catch (...) {
    generateError(TRI_ERROR_INTERNAL);
    return false;
  }
 
  // unreachable
  TRI_ASSERT(false);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief reads a single a document, coordinator case in a cluster
////////////////////////////////////////////////////////////////////////////////

bool RestMvccDocumentHandler::getAllDocumentsCoordinator (string const& collname,
                                                          string const& returnType) {
  string const& dbname = _request->databaseName();

  triagens::rest::HttpResponse::HttpResponseCode responseCode;
  string contentType;
  string resultBody;

  int error = triagens::arango::getAllDocumentsOnCoordinator(dbname, 
                                                             collname, 
                                                             returnType, 
                                                             responseCode, 
                                                             contentType, 
                                                             resultBody);

  if (error != TRI_ERROR_NO_ERROR) {
    generateTransactionError(collname, error);
    return false;
  }
  // Return the response we got:
  _response = createResponse(responseCode);
  _response->setContentType(contentType);
  _response->body().appendText(resultBody.c_str(), resultBody.size());
  return responseCode >= triagens::rest::HttpResponse::BAD;
}

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock REST_DOCUMENT_READ_HEAD
/// @brief reads a single document head
///
/// @RESTHEADER{HEAD /_api/document/{document-handle},Read document header}
///
/// @RESTURLPARAMETERS
///
/// @RESTURLPARAM{document-handle,string,required}
/// The handle of the document.
///
/// @RESTQUERYPARAMETERS
///
/// @RESTQUERYPARAM{rev,string,optional}
/// You can conditionally fetch a document based on a target revision id by
/// using the *rev* URL parameter.
///
/// @RESTHEADERPARAMETERS
///
/// @RESTHEADERPARAM{If-None-Match,string,optional}
/// If the "If-None-Match" header is given, then it must contain exactly one
/// etag. If the current document revision is different to the specified etag,
/// an *HTTP 200* response is returned. If the current document revision is
/// identical to the specified etag, then an *HTTP 304* is returned.
///
/// @RESTHEADERPARAM{If-Match,string,optional}
/// You can conditionally fetch a document based on a target revision id by
/// using the *if-match* HTTP header.
///
/// @RESTDESCRIPTION
/// Like *GET*, but only returns the header fields and not the body. You
/// can use this call to get the current revision of a document or check if
/// the document was deleted.
///
/// @RESTRETURNCODES
///
/// @RESTRETURNCODE{200}
/// is returned if the document was found
///
/// @RESTRETURNCODE{304}
/// is returned if the "If-None-Match" header is given and the document has
/// same version
///*
/// @RESTRETURNCODE{404}
/// is returned if the document or collection was not found
///
/// @RESTRETURNCODE{412}
/// is returned if a "If-Match" header or *rev* is given and the found
/// document has a different version. The response will also contain the found
/// document's current revision in the *etag* header.
///
/// @EXAMPLES
///
/// @EXAMPLE_ARANGOSH_RUN{RestMvccDocumentHandlerReadDocumentHead}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///
///     var document = db.products.save({"hello":"world"});
///     var url = "/_api/document/" + document._id;
///
///     var response = logCurlRequest('HEAD', url);
///
///     assert(response.code === 200);
/// @END_EXAMPLE_ARANGOSH_RUN
/// @endDocuBlock
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

bool RestMvccDocumentHandler::checkDocument () {
  vector<string> const& suffix = _request->suffix();

  if (suffix.size() != 2) {
    generateError(HttpResponse::BAD,
                  TRI_ERROR_HTTP_BAD_PARAMETER,
                  "expecting URI /_api/document/<document-handle>");
    return false;
  }

  return readSingleDocument(false);
}

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock REST_DOCUMENT_REPLACE
/// @brief replaces a document
///
/// @RESTHEADER{PUT /_api/document/{document-handle},Replace document}
///
/// @RESTBODYPARAM{document,json,required}
/// A JSON representation of the new document.
///
/// @RESTURLPARAMETERS
///
/// @RESTURLPARAM{document-handle,string,required}
/// The handle of the document.
///
/// @RESTQUERYPARAMETERS
///
/// @RESTQUERYPARAM{waitForSync,boolean,optional}
/// Wait until document has been synced to disk.
///
/// @RESTQUERYPARAM{rev,string,optional}
/// You can conditionally replace a document based on a target revision id by
/// using the *rev* URL parameter.
///
/// @RESTQUERYPARAM{policy,string,optional}
/// To control the update behavior in case there is a revision mismatch, you
/// can use the *policy* parameter (see below).
///
/// @RESTHEADERPARAMETERS
///
/// @RESTHEADERPARAM{If-Match,string,optional}
/// You can conditionally replace a document based on a target revision id by
/// using the *if-match* HTTP header.
///
/// @RESTDESCRIPTION
/// Completely updates (i.e. replaces) the document identified by *document-handle*.
/// If the document exists and can be updated, then a *HTTP 201* is returned
/// and the "ETag" header field contains the new revision of the document.
///
/// If the new document passed in the body of the request contains the
/// *document-handle* in the attribute *_id* and the revision in *_rev*,
/// these attributes will be ignored. Only the URI and the "ETag" header are
/// relevant in order to avoid confusion when using proxies.
///
/// Optionally, the URL parameter *waitForSync* can be used to force
/// synchronisation of the document replacement operation to disk even in case
/// that the *waitForSync* flag had been disabled for the entire collection.
/// Thus, the *waitForSync* URL parameter can be used to force synchronisation
/// of just specific operations. To use this, set the *waitForSync* parameter
/// to *true*. If the *waitForSync* parameter is not specified or set to
/// *false*, then the collection's default *waitForSync* behavior is
/// applied. The *waitForSync* URL parameter cannot be used to disable
/// synchronisation for collections that have a default *waitForSync* value
/// of *true*.
///
/// The body of the response contains a JSON object with the information about
/// the handle and the revision. The attribute *_id* contains the known
/// *document-handle* of the updated document, *_key* contains the key which 
/// uniquely identifies a document in a given collection, and the attribute *_rev*
/// contains the new document revision.
///
/// If the document does not exist, then a *HTTP 404* is returned and the
/// body of the response contains an error document.
///
/// There are two ways for specifying the targeted document revision id for
/// conditional replacements (i.e. replacements that will only be executed if
/// the revision id found in the database matches the document revision id specified
/// in the request):
/// - specifying the target revision in the *rev* URL query parameter
/// - specifying the target revision in the *if-match* HTTP header
///
/// Specifying a target revision is optional, however, if done, only one of the
/// described mechanisms must be used (either the *rev* URL parameter or the
/// *if-match* HTTP header).
/// Regardless which mechanism is used, the parameter needs to contain the target
/// document revision id as returned in the *_rev* attribute of a document or
/// by an HTTP *etag* header.
///
/// For example, to conditionally replace a document based on a specific revision
/// id, you can use the following request:
///
/// `PUT /_api/document/document-handle?rev=etag`
///
/// If a target revision id is provided in the request (e.g. via the *etag* value
/// in the *rev* URL query parameter above), ArangoDB will check that
/// the revision id of the document found in the database is equal to the target
/// revision id provided in the request. If there is a mismatch between the revision
/// id, then by default a *HTTP 412* conflict is returned and no replacement is
/// performed.
///
/// The conditional update behavior can be overriden with the *policy* URL query parameter:
///
/// `PUT /_api/document/document-handle?policy=policy`
///
/// If *policy* is set to *error*, then the behavior is as before: replacements
/// will fail if the revision id found in the database does not match the target
/// revision id specified in the request.
///
/// If *policy* is set to *last*, then the replacement will succeed, even if the
/// revision id found in the database does not match the target revision id specified
/// in the request. You can use the *last* *policy* to force replacements.
///
/// @RESTRETURNCODES
///
/// @RESTRETURNCODE{201}
/// is returned if the document was replaced successfully and *waitForSync* was
/// *true*.
///
/// @RESTRETURNCODE{202}
/// is returned if the document was replaced successfully and *waitForSync* was
/// *false*.
///
/// @RESTRETURNCODE{400}
/// is returned if the body does not contain a valid JSON representation of a
/// document.  The response body contains an error document in this case.
///
/// @RESTRETURNCODE{404}
/// is returned if the collection or the document was not found
///
/// @RESTRETURNCODE{412}
/// is returned if a "If-Match" header or *rev* is given and the found
/// document has a different version. The response will also contain the found
/// document's current revision in the *_rev* attribute. Additionally, the
/// attributes *_id* and *_key* will be returned.
///
/// @EXAMPLES
///
/// Using document handle:
///
/// @EXAMPLE_ARANGOSH_RUN{RestMvccDocumentHandlerUpdateDocument}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///
///     var document = db.products.save({"hello":"world"});
///     var url = "/_api/document/" + document._id;
///
///     var response = logCurlRequest('PUT', url, '{"Hello": "you"}');
///
///     assert(response.code === 202);
///
///     logJsonResponse(response);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Unknown document handle:
///
/// @EXAMPLE_ARANGOSH_RUN{RestMvccDocumentHandlerUpdateDocumentUnknownHandle}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///
///     var document = db.products.save({"hello":"world"});
///     db.products.remove(document._id);
///     var url = "/_api/document/" + document._id;
///
///     var response = logCurlRequest('PUT', url, "{}");
///
///     assert(response.code === 404);
///
///     logJsonResponse(response);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Produce a revision conflict:
///
/// @EXAMPLE_ARANGOSH_RUN{RestMvccDocumentHandlerUpdateDocumentIfMatchOther}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///
///     var document = db.products.save({"hello":"world"});
///     var document2 = db.products.save({"hello2":"world"});
///     var url = "/_api/document/" + document._id;
///     var headers = {"If-Match":  "\"" + document2._rev + "\""};
///
///     var response = logCurlRequest('PUT', url, '{"other":"content"}', headers);
///
///     assert(response.code === 412);
///
///     logJsonResponse(response);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Last write wins:
///
/// @EXAMPLE_ARANGOSH_RUN{RestMvccDocumentHandlerUpdateDocumentIfMatchOtherLastWriteWins}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///
///     var document = db.products.save({"hello":"world"});
///     var document2 = db.products.replace(document._id,{"other":"content"});
///     var url = "/_api/document/products/" + document._rev + "?policy=last";
///     var headers = {"If-Match":  "\"" + document2._rev + "\""};
///
///     var response = logCurlRequest('PUT', url, "{}", headers);
///     assert(response.code === 202);
///
///     logJsonResponse(response);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Alternative to header field:
///
/// @EXAMPLE_ARANGOSH_RUN{RestMvccDocumentHandlerUpdateDocumentRevOther}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///
///     var document = db.products.save({"hello":"world"});
///     var document2 = db.products.save({"hello2":"world"});
///     var url = "/_api/document/" + document._id + "?rev=" + document2._rev;
///
///     var response = logCurlRequest('PUT', url, '{"other":"content"}');
///
///     assert(response.code === 412);
///
///     logJsonResponse(response);
/// @END_EXAMPLE_ARANGOSH_RUN
/// @endDocuBlock  
////////////////////////////////////////////////////////////////////////////////

bool RestMvccDocumentHandler::replaceDocument () {
  return modifyDocument(false);
}

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock REST_DOCUMENT_UPDATE
/// @brief updates a document
///
/// @RESTHEADER{PATCH /_api/document/{document-handle}, Patch document}
///
/// @RESTBODYPARAM{document,json,required}
/// A JSON representation of the document update.
///
/// @RESTURLPARAMETERS
///
/// @RESTURLPARAM{document-handle,string,required}
/// The handle of the document.
///
/// @RESTQUERYPARAMETERS
///
/// @RESTQUERYPARAM{keepNull,boolean,optional}
/// If the intention is to delete existing attributes with the patch command,
/// the URL query parameter *keepNull* can be used with a value of *false*.
/// This will modify the behavior of the patch command to remove any attributes
/// from the existing document that are contained in the patch document with an
/// attribute value of *null*.
///
/// @RESTQUERYPARAM{mergeObjects,boolean,optional}
/// Controls whether objects (not arrays) will be merged if present in both the
/// existing and the patch document. If set to *false*, the value in the
/// patch document will overwrite the existing document's value. If set to
/// *true*, objects will be merged. The default is *true*.
///
/// @RESTQUERYPARAM{waitForSync,boolean,optional}
/// Wait until document has been synced to disk.
///
/// @RESTQUERYPARAM{rev,string,optional}
/// You can conditionally patch a document based on a target revision id by
/// using the *rev* URL parameter.
///
/// @RESTQUERYPARAM{policy,string,optional}
/// To control the update behavior in case there is a revision mismatch, you
/// can use the *policy* parameter.
///
/// @RESTHEADERPARAMETERS
///
/// @RESTHEADERPARAM{If-Match,string,optional}
/// You can conditionally patch a document based on a target revision id by
/// using the *if-match* HTTP header.
///
/// @RESTDESCRIPTION
/// Partially updates the document identified by *document-handle*.
/// The body of the request must contain a JSON document with the attributes
/// to patch (the patch document). All attributes from the patch document will
/// be added to the existing document if they do not yet exist, and overwritten
/// in the existing document if they do exist there.
///
/// Setting an attribute value to *null* in the patch document will cause a
/// value of *null* be saved for the attribute by default.
///
/// Optionally, the URL parameter *waitForSync* can be used to force
/// synchronisation of the document update operation to disk even in case
/// that the *waitForSync* flag had been disabled for the entire collection.
/// Thus, the *waitForSync* URL parameter can be used to force synchronisation
/// of just specific operations. To use this, set the *waitForSync* parameter
/// to *true*. If the *waitForSync* parameter is not specified or set to
/// *false*, then the collection's default *waitForSync* behavior is
/// applied. The *waitForSync* URL parameter cannot be used to disable
/// synchronisation for collections that have a default *waitForSync* value
/// of *true*.
///
/// The body of the response contains a JSON object with the information about
/// the handle and the revision. The attribute *_id* contains the known
/// *document-handle* of the updated document, *_key* contains the key which 
/// uniquely identifies a document in a given collection, and the attribute *_rev*
/// contains the new document revision.
///
/// If the document does not exist, then a *HTTP 404* is returned and the
/// body of the response contains an error document.
///
/// You can conditionally update a document based on a target revision id by
/// using either the *rev* URL parameter or the *if-match* HTTP header.
/// To control the update behavior in case there is a revision mismatch, you
/// can use the *policy* parameter. This is the same as when replacing
/// documents (see replacing documents for details).
///
/// @RESTRETURNCODES
///
/// @RESTRETURNCODE{201}
/// is returned if the document was created successfully and *waitForSync* was
/// *true*.
///
/// @RESTRETURNCODE{202}
/// is returned if the document was created successfully and *waitForSync* was
/// *false*.
///
/// @RESTRETURNCODE{400}
/// is returned if the body does not contain a valid JSON representation of a
/// document. The response body contains an error document in this case.
///
/// @RESTRETURNCODE{404}
/// is returned if the collection or the document was not found
///
/// @RESTRETURNCODE{412}
/// is returned if a "If-Match" header or *rev* is given and the found
/// document has a different version. The response will also contain the found
/// document's current revision in the *_rev* attribute. Additionally, the
/// attributes *_id* and *_key* will be returned.
///
/// @EXAMPLES
///
/// patches an existing document with new content.
///
/// @EXAMPLE_ARANGOSH_RUN{RestMvccDocumentHandlerPatchDocument}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///
///     var document = db.products.save({"one":"world"});
///     var url = "/_api/document/" + document._id;
///
///     var response = logCurlRequest("PATCH", url, { "hello": "world" });
///
///     assert(response.code === 202);
///
///     logJsonResponse(response);
///     var response2 = logCurlRequest("PATCH", url, { "numbers": { "one": 1, "two": 2, "three": 3, "empty": null } });
///     assert(response2.code === 202);
///     logJsonResponse(response2);
///     var response3 = logCurlRequest("GET", url);
///     assert(response3.code === 200);
///     logJsonResponse(response3);
///     var response4 = logCurlRequest("PATCH", url + "?keepNull=false", { "hello": null, "numbers": { "four": 4 } });
///     assert(response4.code === 202);
///     logJsonResponse(response4);
///     var response5 = logCurlRequest("GET", url);
///     assert(response5.code === 200);
///     logJsonResponse(response5);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Merging attributes of an object using `mergeObjects`:
///
/// @EXAMPLE_ARANGOSH_RUN{RestMvccDocumentHandlerPatchDocumentMerge}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///
///     var document = db.products.save({"inhabitants":{"china":1366980000,"india":1263590000,"usa":319220000}});
///     var url = "/_api/document/" + document._id;
///
///     var response = logCurlRequest("GET", url);
///     assert(response.code === 200);
///     logJsonResponse(response);
///
///     var response = logCurlRequest("PATCH", url + "?mergeObjects=true", { "inhabitants": {"indonesia":252164800,"brazil":203553000 }});
///     assert(response.code === 202);
///
///     var response2 = logCurlRequest("GET", url);
///     assert(response2.code === 200);
///     logJsonResponse(response2);
///
///     var response3 = logCurlRequest("PATCH", url + "?mergeObjects=false", { "inhabitants": { "pakistan":188346000 }});
///     assert(response3.code === 202);
///     logJsonResponse(response3);
///
///     var response4 = logCurlRequest("GET", url);
///     assert(response4.code === 200);
///     logJsonResponse(response4);
/// @END_EXAMPLE_ARANGOSH_RUN
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

bool RestMvccDocumentHandler::updateDocument () {
  return modifyDocument(true);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief helper function for replaceDocument and updateDocument
////////////////////////////////////////////////////////////////////////////////

bool RestMvccDocumentHandler::modifyDocument (bool isPatch) {
  vector<string> const& suffix = _request->suffix();

  if (suffix.size() != 2) {
    string msg("expecting ");
    msg.append(isPatch ? "PATCH" : "PUT");
    msg.append(" /_api/document/<document-handle>");

    generateError(HttpResponse::BAD,
                  TRI_ERROR_HTTP_BAD_PARAMETER,
                  msg);
    return false;
  }

  // split the document reference
  string const& collectionName = suffix[0];
  string const& key            = suffix[1];

  std::unique_ptr<TRI_json_t> json(parseJsonBody());
  
  if (json.get() == nullptr) {
    generateError(TRI_ERROR_OUT_OF_MEMORY);
    return false;
  }

  if (json.get()->_type != TRI_JSON_OBJECT) {
    generateError(TRI_ERROR_ARANGO_DOCUMENT_TYPE_INVALID);
    return false;
  }
  
  // need a fake old transaction in order to not throw - can be removed later       
  TransactionBase oldTrx(true);

  // extract the revision
  bool isValidRevision;
  TRI_voc_rid_t const revision = extractRevision("if-match", "rev", isValidRevision);

  if (! isValidRevision) {
    generateError(HttpResponse::BAD,
                  TRI_ERROR_HTTP_BAD_PARAMETER,
                  "invalid revision number");
    return false;
  }

  if (ServerState::instance()->isCoordinator()) {
    // json will be freed inside
    return modifyDocumentCoordinator(collectionName, key, revision, extractUpdatePolicy(), extractBoolValue("waitForSync", false), isPatch, json.get());
  }

  try {
    triagens::mvcc::OperationOptions options;
    
    triagens::mvcc::TransactionScope transactionScope(_vocbase, triagens::mvcc::TransactionScope::NoCollections());
    auto* transaction = transactionScope.transaction();
    auto* transactionCollection = transaction->collection(collectionName);
      
    if (isPatch) {
      // patching an existing document

      // read extra options
      options.keepNull = extractBoolValue("keepNull", true);
      options.keepNull = extractBoolValue("mergeObjects", true);

      auto document = triagens::mvcc::Document::CreateFromKey(key, revision);
      auto updateResult = triagens::mvcc::CollectionOperations::UpdateDocument(&transactionScope, transactionCollection, document, json.get(), options);

      if (updateResult.code != TRI_ERROR_NO_ERROR) {
        THROW_ARANGO_EXCEPTION(updateResult.code);
      }
      generate20x(insertResponseCode(updateResult.waitForSync), transactionCollection, &updateResult);
    }
    else {
      auto document = triagens::mvcc::Document::CreateFromJson(transactionCollection->shaper(), json.get(), key, revision);
      auto replaceResult = triagens::mvcc::CollectionOperations::ReplaceDocument(&transactionScope, transactionCollection, document, options);

      if (replaceResult.code != TRI_ERROR_NO_ERROR) {
        THROW_ARANGO_EXCEPTION(replaceResult.code);
      }

      generate20x(insertResponseCode(replaceResult.waitForSync), transactionCollection, &replaceResult);
    }

    return true;
  }
  catch (triagens::arango::Exception const& ex) { 
    generateError(HttpResponse::responseCode(ex.code()), ex.code(), ex.what());
    return false;
  }
  catch (...) {
    generateError(TRI_ERROR_INTERNAL);
    return false;
  }

  // unreachable
  TRI_ASSERT(false);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief modifies a document, coordinator case in a cluster
////////////////////////////////////////////////////////////////////////////////

bool RestMvccDocumentHandler::modifyDocumentCoordinator (
                              string const& collname,
                              string const& key,
                              TRI_voc_rid_t const rev,
                              TRI_doc_update_policy_e policy,
                              bool waitForSync,
                              bool isPatch,
                              TRI_json_t* json) {
  string const& dbname = _request->databaseName();
  map<string, string> headers = triagens::arango::getForwardableRequestHeaders(_request);
  triagens::rest::HttpResponse::HttpResponseCode responseCode;
  map<string, string> resultHeaders;
  string resultBody;

  bool keepNull = true;
  if (! strcmp(_request->value("keepNull"), "false")) {
    keepNull = false;
  }
  bool mergeObjects = true;
  if (TRI_EqualString(_request->value("mergeObjects"), "false")) {
    mergeObjects = false;
  }

  int error = triagens::arango::modifyDocumentOnCoordinator(
            dbname, collname, key, rev, policy, waitForSync, isPatch,
            keepNull, mergeObjects, json, headers, responseCode, resultHeaders, resultBody);

  if (error != TRI_ERROR_NO_ERROR) {
    generateTransactionError(collname, error);
    return false;
  }

  // Essentially return the response we got from the DBserver, be it
  // OK or an error:
  _response = createResponse(responseCode);
  triagens::arango::mergeResponseHeaders(_response, resultHeaders);
  _response->body().appendText(resultBody.c_str(), resultBody.size());
  return responseCode >= triagens::rest::HttpResponse::BAD;
}

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock REST_DOCUMENT_DELETE
/// @brief deletes a document
///
/// @RESTHEADER{DELETE /_api/document/{document-handle}, Deletes document}
///
/// @RESTURLPARAMETERS
///
/// @RESTURLPARAM{document-handle,string,required}
/// Deletes the document identified by *document-handle*.
///
/// @RESTQUERYPARAMETERS
///
/// @RESTQUERYPARAM{rev,string,optional}
/// You can conditionally delete a document based on a target revision id by
/// using the *rev* URL parameter.
///
/// @RESTQUERYPARAM{policy,string,optional}
/// To control the update behavior in case there is a revision mismatch, you
/// can use the *policy* parameter. This is the same as when replacing
/// documents (see replacing documents for more details).
///
/// @RESTQUERYPARAM{waitForSync,boolean,optional}
/// Wait until document has been synced to disk.
///
/// @RESTHEADERPARAMETERS
///
/// @RESTHEADERPARAM{If-Match,string,optional}
/// You can conditionally delete a document based on a target revision id by
/// using the *if-match* HTTP header.
///
/// @RESTDESCRIPTION
/// The body of the response contains a JSON object with the information about
/// the handle and the revision. The attribute *_id* contains the known
/// *document-handle* of the deleted document, *_key* contains the key which 
/// uniquely identifies a document in a given collection, and the attribute *_rev*
/// contains the new document revision.
///
/// If the *waitForSync* parameter is not specified or set to
/// *false*, then the collection's default *waitForSync* behavior is
/// applied. The *waitForSync* URL parameter cannot be used to disable
/// synchronisation for collections that have a default *waitForSync* value
/// of *true*.
///
/// @RESTRETURNCODES
///
/// @RESTRETURNCODE{200}
/// is returned if the document was deleted successfully and *waitForSync* was
/// *true*.
///
/// @RESTRETURNCODE{202}
/// is returned if the document was deleted successfully and *waitForSync* was
/// *false*.
///
/// @RESTRETURNCODE{404}
/// is returned if the collection or the document was not found.
/// The response body contains an error document in this case.
///
/// @RESTRETURNCODE{412}
/// is returned if a "If-Match" header or *rev* is given and the found
/// document has a different version. The response will also contain the found
/// document's current revision in the *_rev* attribute. Additionally, the
/// attributes *_id* and *_key* will be returned.
///
/// @EXAMPLES
///
/// Using document handle:
///
/// @EXAMPLE_ARANGOSH_RUN{RestMvccDocumentHandlerDeleteDocument}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn, { waitForSync: true });
///     var document = db.products.save({"hello":"world"});
///
///     var url = "/_api/document/" + document._id;
///
///     var response = logCurlRequest('DELETE', url);
///
///     assert(response.code === 200);
///
///     logJsonResponse(response);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Unknown document handle:
///
/// @EXAMPLE_ARANGOSH_RUN{RestMvccDocumentHandlerDeleteDocumentUnknownHandle}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn, { waitForSync: true });
///     var document = db.products.save({"hello":"world"});
///     db.products.remove(document._id);
///
///     var url = "/_api/document/" + document._id;
///
///     var response = logCurlRequest('DELETE', url);
///
///     assert(response.code === 404);
///
///     logJsonResponse(response);
/// @END_EXAMPLE_ARANGOSH_RUN
///
/// Revision conflict:
///
/// @EXAMPLE_ARANGOSH_RUN{RestMvccDocumentHandlerDeleteDocumentIfMatchOther}
///     var cn = "products";
///     db._drop(cn);
///     db._create(cn);
///
///     var document = db.products.save({"hello":"world"});
///     var document2 = db.products.save({"hello2":"world"});
///     var url = "/_api/document/" + document._id;
///     var headers = {"If-Match":  "\"" + document2._rev + "\""};
///
///     var response = logCurlRequest('DELETE', url, "", headers);
///
///     assert(response.code === 412);
///
///     logJsonResponse(response);
/// @END_EXAMPLE_ARANGOSH_RUN
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

bool RestMvccDocumentHandler::removeDocument () {
  vector<string> const& suffix = _request->suffix();

  if (suffix.size() != 2) {
    generateError(HttpResponse::BAD,
                  TRI_ERROR_HTTP_BAD_PARAMETER,
                  "expecting DELETE /_api/document/<document-handle>");
    return false;
  }

  // split the document reference
  string const& collectionName = suffix[0];
  string const& key            = suffix[1];

  // extract the revision
  bool isValidRevision;
  TRI_voc_rid_t const revision = extractRevision("if-match", "rev", isValidRevision);

  if (! isValidRevision) {
    generateError(HttpResponse::BAD,
                  TRI_ERROR_HTTP_BAD_PARAMETER,
                  "invalid revision number");
    return false;
  }

  // extract or choose the update policy
  TRI_doc_update_policy_e const policy = extractUpdatePolicy();

  if (policy == TRI_DOC_UPDATE_ILLEGAL) {
    generateError(HttpResponse::BAD,
                  TRI_ERROR_HTTP_BAD_PARAMETER,
                  "policy must be 'error' or 'last'");
    return false;
  }

  if (ServerState::instance()->isCoordinator()) {
    return removeDocumentCoordinator(collectionName, key, revision, policy, extractBoolValue("waitForSync", false));
  }
  
  // need a fake old transaction in order to not throw - can be removed later       
  TransactionBase oldTrx(true);

  try {
    triagens::mvcc::OperationOptions options;
    options.waitForSync = extractBoolValue("waitForSync", false);

    triagens::mvcc::TransactionScope transactionScope(_vocbase, triagens::mvcc::TransactionScope::NoCollections());
    auto* transaction = transactionScope.transaction();
    auto* transactionCollection = transaction->collection(collectionName);

    auto document = triagens::mvcc::Document::CreateFromKey(key, revision);
    auto removeResult = triagens::mvcc::CollectionOperations::RemoveDocument(&transactionScope, transactionCollection, document, options);

    if (removeResult.code != TRI_ERROR_NO_ERROR) {
      THROW_ARANGO_EXCEPTION(removeResult.code);
    }

    generate20x(removeResponseCode(removeResult.waitForSync), transactionCollection, &removeResult);
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
/// @brief deletes a document, coordinator case in a cluster
////////////////////////////////////////////////////////////////////////////////

bool RestMvccDocumentHandler::removeDocumentCoordinator (string const& collname,
                                                         string const& key,
                                                         TRI_voc_rid_t const rev,
                                                         TRI_doc_update_policy_e policy,
                                                         bool waitForSync) {
  string const& dbname = _request->databaseName();
  triagens::rest::HttpResponse::HttpResponseCode responseCode;
  map<string, string> headers = triagens::arango::getForwardableRequestHeaders(_request);
  map<string, string> resultHeaders;
  string resultBody;

  int error = triagens::arango::deleteDocumentOnCoordinator(
            dbname, collname, key, rev, policy, waitForSync, headers,
            responseCode, resultHeaders, resultBody);

  if (error != TRI_ERROR_NO_ERROR) {
    generateTransactionError(collname, error);
    return false;
  }
  // Essentially return the response we got from the DBserver, be it
  // OK or an error:
  _response = createResponse(responseCode);
  triagens::arango::mergeResponseHeaders(_response, resultHeaders);
  _response->body().appendText(resultBody.c_str(), resultBody.size());
  return responseCode >= triagens::rest::HttpResponse::BAD;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief generates a HTTP 201 or 202 response
////////////////////////////////////////////////////////////////////////////////

void RestMvccDocumentHandler::generate20x (triagens::rest::HttpResponse::HttpResponseCode responseCode,
                                           triagens::mvcc::TransactionCollection* collection,
                                           triagens::mvcc::OperationResult const* result) {
  auto const mptr = result->mptr;

  TRI_ASSERT(mptr->getDataPtr() != nullptr); 
  auto key = TRI_EXTRACT_MARKER_KEY(mptr);
  
#if 0
  // TODO: If we are a DBserver, we want to use the cluster-wide collection
  // name for error reporting:
  string collectionName = collection->name();
  if (triagens::arango::ServerState::instance()->isDBserver()) {
    collectionName = resolver()->getCollectionName(collection->id());
  }
#endif

  string const&& handle = DocumentHelper::assembleDocumentId(collection->name(), key);
  string const&& rev    = std::to_string(mptr->_rid);

  _response = createResponse(responseCode);
  _response->setContentType("application/json; charset=utf-8");

  if (responseCode != HttpResponse::OK) {
    // 200 OK is sent in case of delete or update.
    // in these cases we do not return etag nor location
    _response->setHeader("etag", strlen("etag"), "\"" + rev + "\"");
    // handle does not need to be RFC 2047-encoded

    if (_request->compatibility() < 10400L) {
      // pre-1.4 location header (e.g. /_api/document/xyz)
      _response->setHeader("location", strlen("location"), string(MVCC_DOCUMENT_PATH + "/" + handle));
    }
    else {
      // 1.4+ location header (e.g. /_db/_system/_api/document/xyz)
      _response->setHeader("location", strlen("location"), string("/_db/" + _request->databaseName() + MVCC_DOCUMENT_PATH + "/" + handle));
    }
  }

  // _id and _key are safe and do not need to be JSON-encoded
  _response->body()
    .appendText("{\"error\":false,\"" TRI_VOC_ATTRIBUTE_ID "\":\"")
    .appendText(handle)
    .appendText("\",\"" TRI_VOC_ATTRIBUTE_REV "\":\"")
    .appendText(rev)
    .appendText("\",\"" TRI_VOC_ATTRIBUTE_KEY "\":\"")
    .appendText(key)
    .appendText("\"}");
}

////////////////////////////////////////////////////////////////////////////////
/// @brief generates a 304 not modified response
////////////////////////////////////////////////////////////////////////////////

void RestMvccDocumentHandler::generateNotModified (TRI_voc_rid_t rid) {
  string const&& rev = std::to_string(rid);

  _response = createResponse(HttpResponse::NOT_MODIFIED);
  _response->setHeader("etag", strlen("etag"), "\"" + rev + "\"");
}

////////////////////////////////////////////////////////////////////////////////
/// @brief generates a document response
////////////////////////////////////////////////////////////////////////////////

void RestMvccDocumentHandler::generateDocument (triagens::mvcc::TransactionCollection* collection,
                                                triagens::mvcc::OperationResult const* result,
                                                bool generateBody) {
  auto const mptr = result->mptr;
  void const* ptr = mptr->getDataPtr();

  TRI_ASSERT(ptr != nullptr); 
  auto key = TRI_EXTRACT_MARKER_KEY(mptr);
  
  string const&& handle = DocumentHelper::assembleDocumentId(collection->name(), key);
  string const&& rev    = std::to_string(mptr->_rid);

  _response = createResponse(HttpResponse::OK);
  _response->setContentType("application/json; charset=utf-8");
  _response->setHeader("etag", strlen("etag"), "\"" + rev + "\"");
 
  triagens::basics::Json json(triagens::basics::Json::Object, 5); 
  json(TRI_VOC_ATTRIBUTE_ID, triagens::basics::Json(handle));
  json(TRI_VOC_ATTRIBUTE_REV, triagens::basics::Json(rev));
  json(TRI_VOC_ATTRIBUTE_KEY, triagens::basics::Json(key));

  TRI_df_marker_type_t type = static_cast<TRI_df_marker_t const*>(ptr)->_type;

  if (type == TRI_DOC_MARKER_KEY_EDGE) {
    CollectionNameResolver resolver(_vocbase); // TODO: remove
    auto* marker = static_cast<TRI_doc_edge_key_marker_t const*>(ptr);
    string const&& from = DocumentHelper::assembleDocumentId(resolver.getCollectionNameCluster(marker->_fromCid), string((char*) marker + marker->_offsetFromKey));
    string const&& to = DocumentHelper::assembleDocumentId(resolver.getCollectionNameCluster(marker->_toCid), string((char*) marker +  marker->_offsetToKey));

    json(TRI_VOC_ATTRIBUTE_FROM, triagens::basics::Json(from));
    json(TRI_VOC_ATTRIBUTE_TO, triagens::basics::Json(to));
  }
  else if (type == TRI_WAL_MARKER_EDGE) {
    CollectionNameResolver resolver(_vocbase); // TODO: remove
    auto* marker = static_cast<triagens::wal::edge_marker_t const*>(ptr);
    string const&& from = DocumentHelper::assembleDocumentId(resolver.getCollectionNameCluster(marker->_fromCid), string((char*) marker + marker->_offsetFromKey));
    string const&& to = DocumentHelper::assembleDocumentId(resolver.getCollectionNameCluster(marker->_toCid), string((char*) marker +  marker->_offsetToKey));

    json(TRI_VOC_ATTRIBUTE_FROM, triagens::basics::Json(from));
    json(TRI_VOC_ATTRIBUTE_TO, triagens::basics::Json(to));
  }
  else if (type == TRI_WAL_MARKER_MVCC_EDGE) {
    CollectionNameResolver resolver(_vocbase); // TODO: remove
    auto* marker = static_cast<triagens::wal::mvcc_edge_marker_t const*>(ptr);
    string const&& from = DocumentHelper::assembleDocumentId(resolver.getCollectionNameCluster(marker->_fromCid), string((char*) marker + marker->_offsetFromKey));
    string const&& to = DocumentHelper::assembleDocumentId(resolver.getCollectionNameCluster(marker->_toCid), string((char*) marker +  marker->_offsetToKey));

    json(TRI_VOC_ATTRIBUTE_FROM, triagens::basics::Json(from));
    json(TRI_VOC_ATTRIBUTE_TO, triagens::basics::Json(to));
  }

  // add document identifier to buffer
  TRI_string_buffer_t buffer;

  // convert object to string
  TRI_InitStringBuffer(&buffer, TRI_UNKNOWN_MEM_ZONE);

  TRI_shaped_json_t shapedJson;
  TRI_EXTRACT_SHAPED_JSON_MARKER(shapedJson, ptr); 
  TRI_StringifyAugmentedShapedJson(collection->shaper(), &buffer, &shapedJson, json.json());

  if (generateBody) {
    _response->body().appendText(TRI_BeginStringBuffer(&buffer), TRI_LengthStringBuffer(&buffer));
  }
  else {
    _response->headResponse(TRI_LengthStringBuffer(&buffer));
  }

  TRI_DestroyStringBuffer(&buffer);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief generates a HTTP 412 response
////////////////////////////////////////////////////////////////////////////////

void RestMvccDocumentHandler::generatePreconditionFailed (triagens::mvcc::TransactionCollection* collection,
                                                          triagens::mvcc::OperationResult const* result) {
  auto const mptr = result->mptr;

  TRI_ASSERT(mptr->getDataPtr() != nullptr); 
  auto key = TRI_EXTRACT_MARKER_KEY(mptr);
  
  string const&& handle = DocumentHelper::assembleDocumentId(collection->name(), key);
  string const&& rev    = std::to_string(mptr->_rid);

  _response = createResponse(HttpResponse::PRECONDITION_FAILED);
  _response->setContentType("application/json; charset=utf-8");
  _response->setHeader("etag", strlen("etag"), "\"" + rev + "\"");

  // _id and _key are safe and do not need to be JSON-encoded
  _response->body()
    .appendText("{\"error\":true,\"code\":")
    .appendInteger((int32_t) HttpResponse::PRECONDITION_FAILED)
    .appendText(",\"errorNum\":")
    .appendInteger((int32_t) TRI_ERROR_ARANGO_CONFLICT)
    .appendText(",\"errorMessage\":\"precondition failed\"")
    .appendText(",\"" TRI_VOC_ATTRIBUTE_ID "\":\"")
    .appendText(handle)
    .appendText("\",\"" TRI_VOC_ATTRIBUTE_REV "\":\"")
    .appendText(rev)
    .appendText("\",\"" TRI_VOC_ATTRIBUTE_KEY "\":\"")
    .appendText(key)
    .appendText("\"}");
}

////////////////////////////////////////////////////////////////////////////////
/// @brief generates a HTTP 201 or 202 response, based on waitForSync flag
////////////////////////////////////////////////////////////////////////////////

triagens::rest::HttpResponse::HttpResponseCode RestMvccDocumentHandler::insertResponseCode (bool waitForSync) const {
  return waitForSync ? rest::HttpResponse::CREATED : rest::HttpResponse::ACCEPTED;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief generates a HTTP 200 or 202 response, based on waitForSync flag
////////////////////////////////////////////////////////////////////////////////

triagens::rest::HttpResponse::HttpResponseCode RestMvccDocumentHandler::removeResponseCode (bool waitForSync) const {
  return waitForSync ? rest::HttpResponse::OK : rest::HttpResponse::ACCEPTED;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
