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

#include "RestVocbaseBaseHandler.h"
#include "Basics/JsonHelper.h"
#include "Basics/conversions.h"
#include "Basics/json-utilities.h"
#include "Rest/HttpRequest.h"
#include "RestServer/VocbaseContext.h"
#include "ShapedJson/shaped-json.h"
#include "Utils/DocumentHelper.h"
#include "Utils/transactions.h"
#include "VocBase/document-collection.h"

using namespace std;
using namespace triagens::basics;
using namespace triagens::rest;
using namespace triagens::arango;

// -----------------------------------------------------------------------------
// --SECTION--                                         REST_VOCBASE_BASE_HANDLER
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                  public constants
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief batch path
////////////////////////////////////////////////////////////////////////////////

const string RestVocbaseBaseHandler::BATCH_PATH = "/_api/batch";

////////////////////////////////////////////////////////////////////////////////
/// @brief document path
////////////////////////////////////////////////////////////////////////////////

const string RestVocbaseBaseHandler::DOCUMENT_PATH = "/_api/document";

////////////////////////////////////////////////////////////////////////////////
/// @brief documents import path
////////////////////////////////////////////////////////////////////////////////

const string RestVocbaseBaseHandler::IMPORT_PATH = "/_api/import";

////////////////////////////////////////////////////////////////////////////////
/// @brief document path
////////////////////////////////////////////////////////////////////////////////

const string RestVocbaseBaseHandler::EDGE_PATH = "/_api/edge";

////////////////////////////////////////////////////////////////////////////////
/// @brief replication path
////////////////////////////////////////////////////////////////////////////////

const string RestVocbaseBaseHandler::REPLICATION_PATH = "/_api/replication";

////////////////////////////////////////////////////////////////////////////////
/// @brief upload path
////////////////////////////////////////////////////////////////////////////////

const string RestVocbaseBaseHandler::UPLOAD_PATH = "/_api/upload";

////////////////////////////////////////////////////////////////////////////////
/// @brief name of the queue
////////////////////////////////////////////////////////////////////////////////

const string RestVocbaseBaseHandler::QUEUE_NAME = "STANDARD";

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief constructor
////////////////////////////////////////////////////////////////////////////////

RestVocbaseBaseHandler::RestVocbaseBaseHandler (HttpRequest* request)
  : RestBaseHandler(request),
    _context(static_cast<VocbaseContext*>(request->getRequestContext())),
    _vocbase(_context->getVocbase()),
    _nolockHeaderSet(nullptr) {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destructor
////////////////////////////////////////////////////////////////////////////////

RestVocbaseBaseHandler::~RestVocbaseBaseHandler () {
}

// -----------------------------------------------------------------------------
// --SECTION--                                                 protected methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief return a boolean value from the request
////////////////////////////////////////////////////////////////////////////////

bool RestVocbaseBaseHandler::extractBoolValue (char const* name,
                                               bool defaultValue) const {
  bool found;
  char const* value = _request->value(name, found);

  if (found) {
    return StringUtils::boolean(value);
  }

  return defaultValue;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief check if a collection needs to be created on the fly
///
/// this method will check the "createCollection" attribute of the request. if
/// it is set to true, it will verify that the named collection actually exists.
/// if the collection does not yet exist, it will create it on the fly.
/// if the "createCollection" attribute is not set or set to false, nothing will
/// happen, and the collection name will not be checked
////////////////////////////////////////////////////////////////////////////////

bool RestVocbaseBaseHandler::checkCreateCollection (string const& name,
                                                    TRI_col_type_e type) {
  bool found;
  char const* valueStr = _request->value("createCollection", found);

  if (! found || ! StringUtils::boolean(valueStr)) {
    // "createCollection" parameter not specified
    // "createCollection" parameter specified, but with non-true value
    return true;
  }

  if (ServerState::instance()->isCoordinator() ||
      ServerState::instance()->isDBserver()) {
    // create-collection is not supported in a cluster
    generateError(TRI_ERROR_CLUSTER_UNSUPPORTED);
    return false;
  }

  TRI_vocbase_col_t* collection = TRI_FindCollectionByNameOrCreateVocBase(_vocbase,
                                                                          name.c_str(),
                                                                          type);

  if (collection == nullptr) {
    generateError(TRI_errno());
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief extracts the revision
////////////////////////////////////////////////////////////////////////////////

TRI_voc_rid_t RestVocbaseBaseHandler::extractRevision (char const* header,
                                                       char const* parameter,
                                                       bool& isValid) {
  isValid = true;
  bool found;
  char const* etag = _request->header(header, found);

  if (found) {
    char const* s = etag;
    char const* e = etag + strlen(etag);

    while (s < e && (s[0] == ' ' || s[0] == '\t')) {
      ++s;
    }

    if (s < e && (s[0] == '"' || s[0] == '\'')) {
      ++s;
    }


    while (s < e && (e[-1] == ' ' || e[-1] == '\t')) {
      --e;
    }

    if (s < e && (e[-1] == '"' || e[-1] == '\'')) {
      --e;
    }

    TRI_voc_rid_t rid = TRI_UInt64String2(s, e - s);
    isValid = (TRI_errno() != TRI_ERROR_ILLEGAL_NUMBER);

    return rid;
  }

  if (parameter != nullptr) {
    etag = _request->value(parameter, found);

    if (found) {
      TRI_voc_rid_t rid = TRI_UInt64String(etag);
      isValid = (TRI_errno() != TRI_ERROR_ILLEGAL_NUMBER);
      return rid;
    }
  }

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief extracts the update policy
////////////////////////////////////////////////////////////////////////////////

TRI_doc_update_policy_e RestVocbaseBaseHandler::extractUpdatePolicy () const {
  bool found;
  char const* policy = _request->value("policy", found);

  if (found) {
    if (TRI_CaseEqualString(policy, "error")) {
      return TRI_DOC_UPDATE_ERROR;
    }
    else if (TRI_CaseEqualString(policy, "last")) {
      return TRI_DOC_UPDATE_LAST_WRITE;
    }
    else {
      return TRI_DOC_UPDATE_ILLEGAL;
    }
  }
  else {
    return TRI_DOC_UPDATE_ERROR;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief parses the body
////////////////////////////////////////////////////////////////////////////////

TRI_json_t* RestVocbaseBaseHandler::parseJsonBody () {
  char* errmsg = nullptr;
  TRI_json_t* json = _request->toJson(&errmsg);

  if (json == nullptr) {
    if (errmsg == nullptr) {
      generateError(HttpResponse::BAD,
                    TRI_ERROR_HTTP_CORRUPTED_JSON,
                    "cannot parse json object");
    }
    else {
      generateError(HttpResponse::BAD,
                    TRI_ERROR_HTTP_CORRUPTED_JSON,
                    errmsg);

      TRI_FreeString(TRI_CORE_MEM_ZONE, errmsg);
    }

    return nullptr;
  }

  TRI_ASSERT(errmsg == nullptr);

  if (TRI_HasDuplicateKeyJson(json)) {
    generateError(HttpResponse::BAD,
                  TRI_ERROR_HTTP_CORRUPTED_JSON,
                  "cannot parse json object");
    TRI_FreeJson(TRI_UNKNOWN_MEM_ZONE, json);
    return nullptr;
  }

  return json;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief generates not implemented
////////////////////////////////////////////////////////////////////////////////

void RestVocbaseBaseHandler::generateNotImplemented (string const& path) {
  generateError(HttpResponse::NOT_IMPLEMENTED,
                TRI_ERROR_NOT_IMPLEMENTED,
                "'" + path + "' not implemented");
}

// -----------------------------------------------------------------------------
// --SECTION--                                                           HANDLER
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

string const& RestVocbaseBaseHandler::queue () const {
  return QUEUE_NAME;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief parses a document handle
/// TODO: merge with DocumentHelper::parseDocumentId
////////////////////////////////////////////////////////////////////////////////

int RestVocbaseBaseHandler::parseDocumentId (CollectionNameResolver const* resolver,
                                             string const& handle,
                                             TRI_voc_cid_t& cid,
                                             std::unique_ptr<char[]>& key) {
  TRI_ASSERT(key.get() == nullptr);

  char const* ptr = handle.c_str();
  char const* end = ptr + handle.size();

  if (end - ptr < 3) {
    // minimum length of document id is 3:
    // at least 1 byte for collection name, '/' + at least 1 byte for key
    return TRI_ERROR_ARANGO_DOCUMENT_HANDLE_BAD;
  }

  char const* pos = static_cast<char const*>(memchr(static_cast<void const*>(ptr), TRI_DOCUMENT_HANDLE_SEPARATOR_CHR, handle.size()));

  if (pos == nullptr || pos >= end - 1) {
    // if no '/' is found, the id is invalid
    // if '/' is at the very end, the id is invalid too
    return TRI_ERROR_ARANGO_DOCUMENT_HANDLE_BAD;
  }
  
  // check if the id contains a second '/'
  if (memchr(static_cast<void const*>(pos + 1), TRI_DOCUMENT_HANDLE_SEPARATOR_CHR, end - pos - 1) != nullptr) {
    return TRI_ERROR_ARANGO_DOCUMENT_HANDLE_BAD;
  }

  char const first = *ptr;

  if (first >= '0' && first <= '9') {
    cid = TRI_UInt64String2(ptr, ptr - pos);
  }
  else {
    cid = resolver->getCollectionIdCluster(std::string(ptr, pos - ptr));
  }

  if (cid == 0) {
    return TRI_ERROR_ARANGO_COLLECTION_NOT_FOUND;
  }
  
  size_t length = end - pos - 1;  
  auto buffer = new char[length + 1];
  memcpy(buffer, pos + 1, length);
  buffer[length] = '\0';
  key.reset(buffer);

  return TRI_ERROR_NO_ERROR;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief prepareExecute, to react to X-Arango-Nolock header
////////////////////////////////////////////////////////////////////////////////

void RestVocbaseBaseHandler::prepareExecute () {
  bool found;
  // LOCKING-DEBUG
  // std::cout << "REQ coming in: " << _request->requestType() << ": " <<_request->fullUrl() << std::endl;
  //std::map<std::string, std::string> h = _request->headers();
  //for (auto& hh : h) {
  //  std::cout << hh.first << ": " << hh.second << std::endl;
  //}
  //std::cout << std::endl;
  char const* shardId = _request->header("x-arango-nolock", found);
  if (found) {
    _nolockHeaderSet = new std::unordered_set<std::string>();
    _nolockHeaderSet->insert(std::string(shardId));
    triagens::arango::Transaction::_makeNolockHeaders = _nolockHeaderSet;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief finalizeExecute, to react to X-Arango-Nolock header
////////////////////////////////////////////////////////////////////////////////

void RestVocbaseBaseHandler::finalizeExecute () {
  if (_nolockHeaderSet != nullptr) {
    triagens::arango::Transaction::_makeNolockHeaders = nullptr;
    delete _nolockHeaderSet;
    _nolockHeaderSet = nullptr;
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
