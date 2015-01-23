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
#include "ShapedJson/Legends.h"
#include "ShapedJson/shaped-json.h"
#include "Utils/Exception.h"
#include "VocBase/server.h"
#include "VocBase/vocbase.h"
#include "Wal/LogfileManager.h"
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

Document Document::CreateFromJson (TRI_shaper_t* shaper,
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

Document Document::CreateFromShapedJson (TRI_shaper_t* shaper,
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

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief insert a document
////////////////////////////////////////////////////////////////////////////////

OperationResult CollectionOperations::InsertDocument (Transaction& transaction,
                                                      TransactionCollection& collection,
                                                      Document& document,
                                                      OperationOptions& options) {
  // create a revision for the document
  if (document.revision == 0) {
    document.revision = TRI_NewTickServer();
  }
  
  // valdidate the document _key or create a new one
  CreateOrValidateKey(collection, document);
  
  std::cout << "KEY: " << document.key << ", REVISION: " << document.revision << "\n";

  // create a temporary marker for the document on the heap
  // the marker memory will be freed automatically when we leave this method
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
  // this will automatically release the master pointer when we leave this method 
  // and have not linked the master pointer
  
  // set data pointer to point to the marker just created
  MasterpointerContainer mptr = collection.masterpointerManager()->create(marker.get()->mem(), transaction.id()());
  
  std::cout << "MARKER: " << marker.get() << "\n";
  std::cout << "MARKER TYPE: " << static_cast<TRI_df_marker_t const*>((*mptr)->getDataPtr())->_type << "\n";
  std::cout << "MPTR: " << *mptr << "\n";


  // acquire a read-lock on the list of indexes so no one else creates or drops indexes
  // while the insert operation is ongoing
  IndexUser indexUser(collection.documentCollection());

  // iterate over all indexes while holding the index read-lock
  size_t i = 0;
  for (auto index : indexUser.indexes()) {
    // and call insert for each index
    try {
      index->insert(&collection, *mptr);
    }
    catch (...) {
      // an error occurred during insert
      // now revert the operation in all indexes 
      while (i > 0) {
        --i;
        try {
          index->forget(&collection, const_cast<TRI_doc_mptr_t const*>(*mptr));
        }
        catch (...) {
        }
      }
      throw;
    }

    // next index
    ++i;
  }

  // when we are here, the document is present in all indexes 

  // now append the marker to the WAL
  OperationResult result;
  result.code = WriteMarker(marker.get(), collection, result);

  if (result.code == TRI_ERROR_NO_ERROR) {
    // make the master pointer point to the WAL location
    mptr.setDataPtr(result.data);
  }

  // do a final sync by clicking each index' lock
  for (auto index : indexUser.indexes()) {
    index->clickLock();
  }
  
  // link the master pointer in the list of master pointers of the collection 
  if (result.code == TRI_ERROR_NO_ERROR) {
    mptr.link();
    result.mptr = *(*mptr);
  }

  std::cout << "INSERTED DOCUMENT. RESULT CODE: " << result.code << ", TICK: " << result.tick << ", FID: " << result.logfileId << "\n";
  return result;
}

////////////////////////////////////////////////////////////////////////////////
/// @brief remove a document
////////////////////////////////////////////////////////////////////////////////

OperationResult CollectionOperations::RemoveDocument (Transaction& transaction,
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
  
void CollectionOperations::CreateOrValidateKey (TransactionCollection& collection,
                                                Document& document) {                   
  if (! document.keySpecified) {
    TRI_ASSERT(document.key.empty());

    // no key specified. now let the collection create one
    document.key.assign(collection.generateKey(document.revision));

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
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief append a marker to the WAL
////////////////////////////////////////////////////////////////////////////////

int CollectionOperations::WriteMarker (triagens::wal::Marker* marker,
                                       TransactionCollection& collection,
                                       OperationResult& result) {
  auto logfileManager = triagens::wal::LogfileManager::instance();
  
  char* oldmarker = static_cast<char*>(marker->mem());
  auto oldm = reinterpret_cast<triagens::wal::document_marker_t*>(oldmarker);

  if ((oldm->_type == TRI_WAL_MARKER_DOCUMENT ||
       oldm->_type == TRI_WAL_MARKER_EDGE) &&
      ! logfileManager->suppressShapeInformation()) {
    // In this case we have to take care of the legend, we know that the
    // marker does not have a legend so far, so first try to get away 
    // with this:
    // (Note that the latter also works for edges!
    TRI_voc_cid_t const cid   = oldm->_collectionId;
    TRI_shape_sid_t const sid = oldm->_shape;
    void* oldLegend;
    triagens::wal::SlotInfoCopy slotInfo = logfileManager->allocateAndWrite(oldmarker, marker->size(), false, cid, sid, 0, oldLegend);

    if (slotInfo.errorCode == TRI_ERROR_LEGEND_NOT_IN_WAL_FILE) {
      // Oh dear, we have to build a legend and patch the marker:
      TRI_document_collection_t* documentCollection = collection.documentCollection();
      triagens::basics::JsonLegend legend(documentCollection->getShaper());  // PROTECTED by trx in trxCollection

      int res = legend.addShape(sid, oldmarker + oldm->_offsetJson, oldm->_size - oldm->_offsetJson);

      if (res != TRI_ERROR_NO_ERROR) {
        return res;
      }

      int64_t sizeChanged = legend.getSize() - (oldm->_offsetJson - oldm->_offsetLegend);
      TRI_voc_size_t newMarkerSize = static_cast<TRI_voc_size_t>(oldm->_size + sizeChanged);

      // Now construct the new marker on the heap:
      char* newmarker = new char[newMarkerSize];
      memcpy(newmarker, oldmarker, oldm->_offsetLegend);
      try {
        legend.dump(newmarker + oldm->_offsetLegend);
      }
      catch (...) {
        delete[] newmarker;
        throw;
      }
      memcpy(newmarker + oldm->_offsetLegend + legend.getSize(), oldmarker + oldm->_offsetJson, oldm->_size - oldm->_offsetJson);

      // And fix its entries:
      auto newm = reinterpret_cast<triagens::wal::document_marker_t*>(newmarker);
      newm->_size = newMarkerSize;
      newm->_offsetJson = (uint32_t) (oldm->_offsetLegend + legend.getSize());

      triagens::wal::SlotInfoCopy slotInfo2 = logfileManager->allocateAndWrite(newmarker, newMarkerSize, false, cid, sid, newm->_offsetLegend, oldLegend);
      delete[] newmarker;

      if (slotInfo2.errorCode != TRI_ERROR_NO_ERROR) {
        return slotInfo2.errorCode;
      }

      result.logfileId = slotInfo2.logfileId;
      result.data      = slotInfo2.mem; 
      result.tick      = slotInfo2.tick;
    }
    else if (slotInfo.errorCode != TRI_ERROR_NO_ERROR) {
      return slotInfo.errorCode;
    }
    else {
      int64_t* legendPtr = reinterpret_cast<int64_t*>(oldmarker + oldm->_offsetLegend);
      *legendPtr =  reinterpret_cast<char*>(oldLegend) - reinterpret_cast<char*>(legendPtr);
      // This means that we can find the old legend relative to
      // the new position in the same WAL file.
      result.logfileId = slotInfo.logfileId;
      result.data      = slotInfo.mem; 
      result.tick      = slotInfo.tick;
    }

  }
  else {  
    // No document or edge marker, just append it to the WAL:
    triagens::wal::SlotInfoCopy slotInfo = logfileManager->allocateAndWrite(marker->mem(), marker->size(), false);

    if (slotInfo.errorCode != TRI_ERROR_NO_ERROR) {
      // some error occurred
      return slotInfo.errorCode;
    }

    result.logfileId = slotInfo.logfileId;
    result.data      = slotInfo.mem;
    result.tick      = slotInfo.tick;
  }

  return TRI_ERROR_NO_ERROR;
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
