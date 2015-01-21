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
#include "Mvcc/Index.h"
#include "Mvcc/IndexUser.h"
#include "Mvcc/MasterpointerManager.h"
#include "ShapedJson/shaped-json.h"
#include "Utils/Exception.h"
#include "VocBase/server.h"
#include "VocBase/vocbase.h"
#include "Wal/Marker.h"

using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                                   struct Document
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create an empty document container
////////////////////////////////////////////////////////////////////////////////

Document::Document (TRI_shaper_t* shaper,
                    TRI_shaped_json_t const* shaped,
                    std::string const& key,
                    TRI_voc_rid_t revision,
                    bool keySpecified,
                    bool freeShape) 
  : shaper(shaper),
    shaped(shaped),
    key(key),
    revision(revision),
    keySpecified(keySpecified),
    freeShape(freeShape) {

  TRI_ASSERT(shaper != nullptr);
  TRI_ASSERT(shaped != nullptr);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a document from another
////////////////////////////////////////////////////////////////////////////////
        
Document::Document (Document&& other) 
  : shaper(other.shaper),
    shaped(other.shaped),
    key(other.key),
    revision(other.revision),
    keySpecified(other.keySpecified),
    freeShape(other.freeShape) {

  // do not double-free
  other.shaped    = nullptr;
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
         
  if (! TRI_IsObjectJson(json)) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_DOCUMENT_TYPE_INVALID);
  }
  
  // check if _key is there
  TRI_json_t const* key = TRI_LookupObjectJson(json, TRI_VOC_ATTRIBUTE_KEY);
  
  if (key != nullptr) {
    if (! TRI_IsStringJson(key)) {
      // _key is there but not a string
      THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_DOCUMENT_KEY_BAD);
    }
  }
  
  auto* shaped = TRI_ShapedJsonJson(shaper, json, true);
  
  if (shaped == nullptr) {
    THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_SHAPER_FAILED);
  }

  if (key != nullptr) {
    // user has specified a key
    return Document(shaper, shaped, std::string(key->_value._string.data, key->_value._string.length - 1), 0, CollectionOperations::KeySpecified, true);
  }

  // user has not specified a key
  return Document(shaper, shaped, std::string(), 0, CollectionOperations::NoKeySpecified, true);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief create a document from ShapedJson
////////////////////////////////////////////////////////////////////////////////

Document Document::createFromShapedJson (TRI_shaper_t* shaper,
                                         TRI_shaped_json_t const* shaped,
                                         char const* key) {

  if (key != nullptr) {         
    // user has specified a key
    return Document(shaper, shaped, std::string(key), 0, CollectionOperations::KeySpecified, false);
  }
     
  // user has not specified a key
  return Document(shaper, shaped, std::string(), 0, CollectionOperations::NoKeySpecified, false);
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
  // first valdidate the document _key or create a new one
  createOrValidateKey(collection, document);
  
  std::cout << "KEY: " << document.key << "\n";

  // create a WAL marker for the document
  std::unique_ptr<triagens::wal::DocumentMarker> marker;
  marker.reset(new triagens::wal::DocumentMarker(transaction.vocbase()->_id,
                                                 collection.id(),
                                                 document.revision,
                                                 transaction.id()(),
                                                 document.key,
                                                 8, /* legendSize */
                                                 document.shaped));
  
  std::cout << "MARKER: " << marker.get() << "\n";
  // create a master pointer which will hold the marker
  MasterpointerContainer mptr = collection.masterpointerManager()->create();

  // set data pointer to point to the marker just created
  mptr.setDataPtr(marker.get()->mem());
  
  std::cout << "MARKER: " << marker.get() << "\n";
  std::cout << "MPTR: " << *mptr << "\n";

std::cout << "MARKER TYPE: " << static_cast<TRI_df_marker_t const*>((*mptr)->getDataPtr())->_type << "\n";
  // acquire a read-lock on the list of indexes so no one else creates or drops indexes
  // while the insert operation is ongoing
  IndexUser indexUser(collection.documentCollection());

  // iterate over all indexes while holding the index read-lock
  for (auto index : indexUser.indexes()) {
    // and call insert for each index
    index->insert(&collection, const_cast<TRI_doc_mptr_t*>(*mptr));
  }

  // do a final sync by clicking each index' lock
  for (auto index : indexUser.indexes()) {
    index->clickLock();
  }

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
/// will throw if the key is invalid
////////////////////////////////////////////////////////////////////////////////
  
void CollectionOperations::createOrValidateKey (TransactionCollection& collection,
                                                Document& document) {                   
  if (! document.keySpecified) {
    TRI_ASSERT(document.key.empty());

    // no key specified. now let the collection create one
    document.key.assign(collection.generateKey(TRI_NewTickServer()));

    if (document.key.empty()) {
      THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_OUT_OF_KEYS);
    }
  }
  else {
    // key was specified, now validate it
    collection.validateKey(document.key); // TODO: handle case isRestore
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
