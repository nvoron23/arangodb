////////////////////////////////////////////////////////////////////////////////
/// @brief V8 Traverser
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2014-2015 ArangoDB GmbH, Cologne, Germany
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
/// @author Michael Hackstein
/// @author Copyright 2014-2015, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2012-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "v8.h"
#include "V8/v8-conv.h"
#include "V8/v8-utils.h"
#include "V8Server/v8-vocbaseprivate.h"
#include "V8Server/v8-wrapshapedjson.h"
#include "V8Server/v8-vocindex.h"
#include "V8Server/v8-collection.h"
#include "Utils/transactions.h"
#include "Utils/V8ResolverGuard.h"
#include "Utils/CollectionNameResolver.h"
#include "Traverser.h"
#include "VocBase/key-generator.h"

using namespace std;
using namespace triagens::basics;
using namespace triagens::arango;

class SimpleEdgeExpander {

  private:
    TRI_edge_direction_e forwardDirection;
    TRI_edge_direction_e backwardDirection;
    TRI_document_collection_t* edgeCollection;
    CollectionNameResolver const* resolver;

  public: 
    void operator() ( Traverser::VertexId source,
                      Traverser::Direction dir,
                      vector<Traverser::Neighbor>& result
                    ) {
      std::vector<TRI_doc_mptr_copy_t> edges;
      // Process Vertex Id!
      size_t split;
      char const* str = source.c_str();
      if (!TRI_ValidateDocumentIdKeyGenerator(str, &split)) {
        // TODO Error Handling
        return;
      }
      std::unique_ptr<char> key;
      string collectionName = string(str, split);
      auto const length = source.size() - split - 1;
      auto buffer = new char[length + 1];
      memcpy(buffer, str + split + 1, length);
      buffer[length] = '\0';
      key.reset(buffer);

      auto col = resolver->getCollectionStruct(collectionName);
      if (col == nullptr) {
        // collection not found
        throw TRI_ERROR_ARANGO_COLLECTION_NOT_FOUND;
      }
      auto collectionCId = col->_cid;
      if (dir == Traverser::FORWARD) {
        edges = TRI_LookupEdgesDocumentCollection(edgeCollection, forwardDirection, collectionCId, key.get());
      } else {
        edges = TRI_LookupEdgesDocumentCollection(edgeCollection, backwardDirection, collectionCId, key.get());
      }

      // TODO Build result
    }
    SimpleEdgeExpander(
      TRI_edge_direction_e edgeDirection,
      TRI_document_collection_t* edgeCollection,
      CollectionNameResolver const* resolver
    ) : 
      edgeCollection(edgeCollection),
      resolver(resolver)
    {
      if (edgeDirection == TRI_EDGE_OUT) {
        forwardDirection = TRI_EDGE_OUT;
        backwardDirection = TRI_EDGE_IN;
      } else if (edgeDirection == TRI_EDGE_IN) {
        forwardDirection = TRI_EDGE_IN;
        backwardDirection = TRI_EDGE_OUT;
      } else {
        forwardDirection = TRI_EDGE_ANY;
        backwardDirection = TRI_EDGE_ANY;
      }
    };
};


struct LocalCollectionGuard {
  LocalCollectionGuard (TRI_vocbase_col_t* collection)
    : _collection(collection) {
  }

  ~LocalCollectionGuard () {
    if (_collection != nullptr && ! _collection->_isLocal) {
      FreeCoordinatorCollection(_collection);
    }
  }

  TRI_vocbase_col_t* _collection;
};

void TRI_RunDijkstraSearch (const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();
  v8::HandleScope scope(isolate);

  if (args.Length() < 4 || args.Length() > 5) {
    TRI_V8_THROW_EXCEPTION_USAGE("AQL_SHORTEST_PATH(<vertexcollection>, <edgecollection>, <start>, <end>, <options>)");
  }

  std::unique_ptr<char[]> key;
  TRI_vocbase_t* vocbase;
  TRI_vocbase_col_t const* col = nullptr;

  vocbase = GetContextVocBase(isolate);

  vector<string> readCollections;
  vector<string> writeCollections;
  TRI_voc_cid_t vertexCollectionCId;
  TRI_voc_cid_t edgeCollectionCId;
  TRI_voc_rid_t rid;

  double lockTimeout = (double) (TRI_TRANSACTION_DEFAULT_LOCK_TIMEOUT / 1000000ULL);
  bool embed = false;
  bool waitForSync = false;

  // get the vertex collection
  if (! args[0]->IsString()) {
    TRI_V8_THROW_TYPE_ERROR("expecting string for <vertexcollection>");
  }
  string vertexCollectionName = TRI_ObjectToString(args[0]);

  // get the edge collection
  if (! args[1]->IsString()) {
    TRI_V8_THROW_TYPE_ERROR("expecting string for <edgecollection>");
  }
  string const edgeCollectionName = TRI_ObjectToString(args[1]);

  vocbase = GetContextVocBase(isolate);

  if (vocbase == nullptr) {
    TRI_V8_THROW_EXCEPTION(TRI_ERROR_ARANGO_DATABASE_NOT_FOUND);
  }
  V8ResolverGuard resolver(vocbase);

  readCollections.push_back(vertexCollectionName);
  readCollections.push_back(edgeCollectionName);
  
  if (! args[2]->IsString()) {
    TRI_V8_THROW_TYPE_ERROR("expecting string for <startVertex>");
  }


  std::string vertexColName;

  if (! ExtractDocumentHandle(isolate, args[2], vertexColName, key, rid)) {
    TRI_V8_THROW_EXCEPTION(TRI_ERROR_ARANGO_DOCUMENT_HANDLE_BAD);
  }

  if (key.get() == nullptr) {
    TRI_V8_THROW_EXCEPTION(TRI_ERROR_ARANGO_DOCUMENT_HANDLE_BAD);
  }

  TRI_ASSERT(key.get() != nullptr);

  // IHHF isCoordinator

  // Start Transaction to collect all parts of the path
  ExplicitTransaction trx(
    vocbase,
    readCollections,
    writeCollections,
    lockTimeout,
    waitForSync,
    embed
  );
  
  int res = trx.begin();

  if (res != TRI_ERROR_NO_ERROR) {
    TRI_V8_THROW_EXCEPTION(res);
  }

  col = resolver.getResolver()->getCollectionStruct(vertexCollectionName);
  if (col == nullptr) {
    // collection not found
    TRI_V8_THROW_EXCEPTION(TRI_ERROR_ARANGO_COLLECTION_NOT_FOUND);
  }
  vertexCollectionCId = col->_cid;
  if (trx.orderBarrier(trx.trxCollection(col->_cid)) == nullptr) {
    TRI_V8_THROW_EXCEPTION_MEMORY();
  }

  col = resolver.getResolver()->getCollectionStruct(edgeCollectionName);
  if (col == nullptr) {
    // collection not found
    TRI_V8_THROW_EXCEPTION(TRI_ERROR_ARANGO_COLLECTION_NOT_FOUND);
  }
  edgeCollectionCId = col->_cid;
  if (trx.orderBarrier(trx.trxCollection(col->_cid)) == nullptr) {
    TRI_V8_THROW_EXCEPTION_MEMORY();
  }

  v8::Handle<v8::Value> result;
  v8::Handle<v8::Array> documents;

  // This is how to get the data out of the collections!
  // Vertices
  TRI_doc_mptr_copy_t document;
  res = trx.readSingle(trx.trxCollection(vertexCollectionCId), &document, key.get());

  // Edges TRI_EDGE_OUT is hardcoded
  TRI_document_collection_t* ecol = trx.trxCollection(edgeCollectionCId)->_collection->_collection;
  std::vector<TRI_doc_mptr_copy_t>&& edges = TRI_LookupEdgesDocumentCollection(ecol, TRI_EDGE_OUT, vertexCollectionCId, key.get());

  // Add Dijkstra here

  // Now build up the result use Subtransactions for each used collection
  if (res == TRI_ERROR_NO_ERROR) {
    {
      // Collect all vertices
      SingleCollectionReadOnlyTransaction subtrx(new V8TransactionContext(true), vocbase, vertexCollectionCId);

      res = subtrx.begin();

      if (res != TRI_ERROR_NO_ERROR) {
        TRI_V8_THROW_EXCEPTION(res);
      }

      result = TRI_WrapShapedJson(isolate, subtrx, vertexCollectionCId, document.getDataPtr());

      if (document.getDataPtr() == nullptr) {
        res = TRI_ERROR_ARANGO_DOCUMENT_NOT_FOUND;
        TRI_V8_THROW_EXCEPTION(res);
      }

      res = subtrx.finish(res);
    }

    {
      // Collect all edges
      SingleCollectionReadOnlyTransaction subtrx2(new V8TransactionContext(true), vocbase, edgeCollectionCId);

      res = subtrx2.begin();

      if (res != TRI_ERROR_NO_ERROR) {
        TRI_V8_THROW_EXCEPTION(res);
      }

      bool error = false;

      uint32_t const n = static_cast<uint32_t>(edges.size());
      documents = v8::Array::New(isolate, static_cast<int>(n));
      for (size_t j = 0;  j < n;  ++j) {
        v8::Handle<v8::Value> doc = TRI_WrapShapedJson(isolate, subtrx2, edgeCollectionCId, edges[j].getDataPtr());

        if (doc.IsEmpty()) {
          error = true;
          break;
        }
        else {
          documents->Set(static_cast<uint32_t>(j), doc);
        }
      }
      if (error) {
        TRI_V8_THROW_EXCEPTION_MEMORY();
      }

      res = subtrx2.finish(res);
    }
  } 
  res = trx.finish(res);

  // Not yet correct. Needs to build an object first.
  // TRI_V8_RETURN(result);
  TRI_V8_RETURN(documents);
}