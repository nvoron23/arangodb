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

#include "CollectionOperations.h"
#include "Basics/json.h"
#include "ShapedJson/shaped-json.h"
#include "Utils/DocumentHelper.h"
#include "Utils/Exception.h"
#include "VocBase/server.h"
#include "VocBase/vocbase.h"

using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                                   struct Document
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create an empty document container
////////////////////////////////////////////////////////////////////////////////

Document::Document (TRI_shaper_t* shaper,
                    TRI_shaped_json_t const* shaped,
                    char const* key,
                    bool freeShape) 
  : shaper(shaper),
    shaped(shaped),
    key(key),
    freeShape(freeShape) {

  TRI_ASSERT(shaper != nullptr);
  TRI_ASSERT(shaped != nullptr);
  // key may be a nullptr
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a document from another
////////////////////////////////////////////////////////////////////////////////
        
Document::Document (Document&& other) 
  : shaper(other.shaper),
    shaped(other.shaped),
    key(other.key),
    freeShape(other.freeShape) {

  // do not double-free
  other.shaped    = nullptr;
  other.key       = nullptr;
  other.freeShape = false;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy a document container
////////////////////////////////////////////////////////////////////////////////

Document::~Document () {
  if (shaped != nullptr && freeShape) {
    TRI_FreeShapedJson(shaper->_memoryZone, const_cast<TRI_shaped_json_t*>(shaped));
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a document from JSON
////////////////////////////////////////////////////////////////////////////////

Document Document::createFromJson (TRI_shaper_t* shaper,
                                   TRI_json_t const* json) {
          
  // extract document key from JSON
  TRI_voc_key_t documentKey = nullptr;
  int res = triagens::arango::DocumentHelper::getKey(json, &documentKey);

  if (res != TRI_ERROR_NO_ERROR) {
    THROW_ARANGO_EXCEPTION(res);
  }
  
  auto* shaped = TRI_ShapedJsonJson(shaper, json, true);
  
  if (shaped == nullptr) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_SHAPER_FAILED);
  }

  return Document(shaper, shaped, static_cast<char const*>(documentKey), true);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a document from ShapedJson
////////////////////////////////////////////////////////////////////////////////

Document Document::createFromShapedJson (TRI_shaper_t* shaper,
                                         TRI_shaped_json_t const* shaped,
                                         char const* key) {
          
  return Document(shaper, shaped, key, false);
}

// -----------------------------------------------------------------------------
// --SECTION--                                       struct CollectionOperations
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief insert a document
////////////////////////////////////////////////////////////////////////////////

OperationResult CollectionOperations::insertDocument (Transaction& transaction,
                                                      TransactionCollection& collection,
                                                      Document& document,
                                                      OperationOptions& options) {

  std::string const&& key = createOrValidateKey(collection, document);
  
  std::cout << "KEY: " << key << "\n";
  OperationResult result;
  std::cout << "INSERT DOCUMENT\n";
  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief remove a document
////////////////////////////////////////////////////////////////////////////////

OperationResult CollectionOperations::removeDocument (Transaction& transaction,
                                                      TransactionCollection& collection,
                                                      Document& document,
                                                      OperationOptions& options) {
  OperationResult result;
  std::cout << "REMOVE DOCUMENT\n";
  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief creates a key if not specified, or validates it if specified
/// returns the key
////////////////////////////////////////////////////////////////////////////////
  
std::string CollectionOperations::createOrValidateKey (TransactionCollection& collection,
                                                       Document& document) {                   
  if (document.key == nullptr) {
    // no key specified. now let the collection create one
    std::string key(collection.generateKey(TRI_NewTickServer()));

    if (key.empty()) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_OUT_OF_KEYS);
    }

    return key;
  }
    
  // key was specified, now validate it
  collection.validateKey(document.key); // TODO: handle case isRestore

  return std::string(document.key);
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
