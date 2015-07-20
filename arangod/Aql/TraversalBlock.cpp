////////////////////////////////////////////////////////////////////////////////
/// @brief Implementation of the ExecutionBlock for Traversals
///
/// @file arangod/Aql/TraversalBlock.cpp
///
/// DISCLAIMER
///
/// Copyright 2010-2014 triagens GmbH, Cologne, Germany
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
/// Copyright holder is triAGENS GmbH, Cologne, Germany
///
/// @author Michael Hackstein
/// @author Copyright 2014, triagens GmbH, Cologne, Germany
/// @author Copyright 2014-2015, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "Aql/TraversalBlock.h"
#include "Utils/ShapedJsonTransformer.h"

using namespace std;
using namespace triagens::basics;
using namespace triagens::arango;
using namespace triagens::aql;

// -----------------------------------------------------------------------------
// --SECTION--                                              class TraversalBlock
// -----------------------------------------------------------------------------

TraversalBlock::TraversalBlock (ExecutionEngine* engine,
                                TraversalNode const* ep)
  : ExecutionBlock(engine, ep),
    _posInPaths(0),
    _useRegister(false),
    _usedConstant(false),
    _vertexReg(0),
    _edgeReg(0),
    _pathReg(0) {

  basics::traverser::TraverserOptions opts;
  ep->fillTraversalOptions(opts);
  std::vector<TRI_document_collection_t*> edgeCollections;
  auto cids = ep->edgeCids();
  for (auto const& cid : cids) {
    edgeCollections.push_back(_trx->documentCollection(cid));
  }
  _traverser.reset(new basics::traverser::DepthFirstTraverser(edgeCollections, opts));
  _resolver = new CollectionNameResolver(_trx->vocbase());
  if (!ep->usesInVariable()) {
    _startId = VertexId();
    std::string vId(ep->_start->getStringValue());
    auto pos = vId.find("/");
    _startId.setCopy(_resolver->getCollectionId(vId.substr(0, pos).c_str()), vId.substr(pos + 1));
  }
  else {
    auto it = ep->getRegisterPlan()->varInfo.find(ep->inVariable()->id);
    TRI_ASSERT(it != ep->getRegisterPlan()->varInfo.end());
    _reg = it->second.registerId;
    _useRegister = true;
  }
  _vertexVar = nullptr;
  _edgeVar = nullptr;
  _pathVar = nullptr;
  if (ep->usesVertexOutVariable()) {
    _vertexVar = ep->vertexOutVariable();
  }

  if (ep->usesEdgeOutVariable()) {
    _edgeVar = ep->edgeOutVariable();
  }

  if (ep->usesPathOutVariable()) {
    _pathVar = ep->pathOutVariable();
  }

  /*
  auto pruner = [] (const TraversalPath<TRI_doc_mptr_copy_t, VertexId>& path) -> bool {
    if (strcmp(path.vertices.back().key, "1") == 0) {
      return true;
    }
    if (strcmp(path.vertices.back().key, "31") == 0) {
      return true;
    }
    return false;
  };
  opts.setPruningFunction(pruner);
  */
}

TraversalBlock::~TraversalBlock () {
  delete _resolver;
  for (auto& p : _paths) {
    p.destroy();
  }
  _paths.clear();
}

int TraversalBlock::initialize () {
  int res = ExecutionBlock::initialize();
  
  auto varInfo = getPlanNode()->getRegisterPlan()->varInfo;
  if (usesVertexOutput()) {
    auto it = varInfo.find(_vertexVar->id);
    TRI_ASSERT(it != varInfo.end());
    TRI_ASSERT(it->second.registerId < ExecutionNode::MaxRegisterId);
    _vertexReg = it->second.registerId;
  }
  if (usesEdgeOutput()) {
    auto it = varInfo.find(_edgeVar->id);
    TRI_ASSERT(it != varInfo.end());
    TRI_ASSERT(it->second.registerId < ExecutionNode::MaxRegisterId);
    _edgeReg = it->second.registerId;
  }
  if (usesPathOutput()) {
    auto it = varInfo.find(_pathVar->id);
    TRI_ASSERT(it != varInfo.end());
    TRI_ASSERT(it->second.registerId < ExecutionNode::MaxRegisterId);
    _pathReg = it->second.registerId;
  }
  

  return res;
}

int TraversalBlock::initializeCursor (AqlItemBlock* items, 
                                      size_t pos) {
  int res = ExecutionBlock::initializeCursor(items, pos);

  if (res != TRI_ERROR_NO_ERROR) {
    return res;
  }

  return TRI_ERROR_NO_ERROR;
}


////////////////////////////////////////////////////////////////////////////////
/// @brief Transform the path to AQLValue object
////////////////////////////////////////////////////////////////////////////////

AqlValue TraversalBlock::pathToAqlValue (
  const TraversalPath<EdgeInfo, VertexId>& p
) {
  Json* path = new Json(Json::Object, 2); 
  Json vertices(Json::Array);
  // TODO FIXME
  for (size_t i = 0; i < p.vertices.size(); ++i) {
    auto collection = _trx->trxCollection(p.vertices[i].cid);
    if (collection == nullptr) {
      // TODO Might be optimized
      SingleCollectionReadOnlyTransaction intTrx(new StandaloneTransactionContext(), _trx->vocbase(), p.vertices[i].cid);
      int res = intTrx.begin();

      if (res != TRI_ERROR_NO_ERROR) {
        THROW_ARANGO_EXCEPTION(res);
      }
      collection = intTrx.trxCollection();
      TRI_doc_mptr_copy_t mptr;
      intTrx.read(&mptr, p.vertices[i].key);
      vertices(TRI_ExpandShapedJson(
        collection->_collection->_collection->getShaper(),
        _resolver,
        p.vertices[i].cid,
        &mptr
      ));
      intTrx.finish(res);
    } else {
      TRI_doc_mptr_copy_t mptr;
      _trx->readSingle(collection, &mptr, p.vertices[i].key);
      vertices(TRI_ExpandShapedJson(
        collection->_collection->_collection->getShaper(),
        _resolver,
        p.vertices[i].cid,
        &mptr
      ));
    }
  }
  Json edges(Json::Array);
  // TODO FIXME
  for (size_t i = 0; i < p.edges.size(); ++i) {
    auto cid = p.edges[i].cid;
    auto collection = _trx->trxCollection(cid);
    TRI_shaped_json_t shapedJson;
    TRI_EXTRACT_SHAPED_JSON_MARKER(shapedJson, &(p.edges[i].mptr));
    edges(TRI_ExpandShapedJson(
      collection->_collection->_collection->getShaper(),
      _resolver,
      cid,
      &p.edges[i].mptr
    ));
  }
  (*path)("vertices", vertices)
         ("edges", edges);

  return AqlValue(path);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief read more paths
////////////////////////////////////////////////////////////////////////////////

bool TraversalBlock::morePaths (size_t hint) {
  for (auto p : _paths) {
    p.destroy();
  }
  _paths.clear();
  _posInPaths = 0;
  if (!_traverser->hasMore()) {
    return false;
  }
  for (size_t j = 0; j < hint; ++j) {
    auto p = _traverser->next();
    if (p.edges.size() == 0) {
      // There are no further paths available.
      break;
    }
    auto tmp = pathToAqlValue(p);
    if (!tmp.isEmpty()) {
      _paths.push_back(tmp);
    }
  }
  return _paths.size() > 0;
}


void TraversalBlock::initializePaths (AqlItemBlock const* items) {
  if (!_useRegister) {
    if (!_usedConstant) {
      _usedConstant = true;
      _traverser->setStartVertex(_startId);
    }
  } else {
    auto in = items->getValueReference(_pos, _reg);
    if (in.isShaped()) {
      auto col = items->getDocumentCollection(_reg);
      VertexId v(col->_info._cid, TRI_EXTRACT_MARKER_KEY(in.getMarker()));
      _traverser->setStartVertex(v);
    } else if (in.isObject()) {
      Json input = in.toJson(nullptr, nullptr);
      if (input.has("_id") ) {
        Json _idJson = input.get("_id");
        if (_idJson.isString()) {
          string _idStr = JsonHelper::getStringValue(_idJson.json(), "");
          VertexId v = triagens::basics::traverser::IdStringToVertexId (
            _resolver,
            _idStr
          );
          _traverser->setStartVertex(v);
        }
      } else if (input.has("vertex")) {
        // This is used whenever the input is the result of another traversal.
        Json vertexJson = input.get("vertex");
        if (vertexJson.has("_id") ) {
          Json _idJson = vertexJson.get("_id");
          if (_idJson.isString()) {
            string _idStr = JsonHelper::getStringValue(_idJson.json(), "");
            VertexId v = triagens::basics::traverser::IdStringToVertexId (
              _resolver,
              _idStr
            );
            _traverser->setStartVertex(v);
          }
        }
      }
    } else {
      std::cout << "FOUND Type: " << in.getTypeString() << std::endl;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief getSome
////////////////////////////////////////////////////////////////////////////////

AqlItemBlock* TraversalBlock::getSome (size_t, // atLeast,
                                       size_t atMost) {

  if (_done) {
    return nullptr;
  }

  if (_buffer.empty()) {

    size_t toFetch = (std::min)(DefaultBatchSize, atMost);
    if (! ExecutionBlock::getBlock(toFetch, toFetch)) {
      _done = true;
      return nullptr;
    }
    _pos = 0;           // this is in the first block
  }

  // If we get here, we do have _buffer.front()
  AqlItemBlock* cur = _buffer.front();
  size_t const curRegs = cur->getNrRegs();

  if (_pos == 0) {
    // Initial initialisation
    initializePaths(cur);
  }

  // Iterate more paths:
  if (_posInPaths >= _paths.size()) {
    if (! morePaths(atMost)) {
      // This input does not have any more paths. maybe the nextone has.
      // we can only return nullptr iff the buffer is empty.
      if (++_pos >= cur->size()) {
        _buffer.pop_front();  // does not throw
        // returnBlock(cur);
        delete cur;
        _pos = 0;
      } else {
        initializePaths(cur);
      }
      return getSome(atMost, atMost);
    }
  }

  size_t available = _paths.size() - _posInPaths;
  size_t toSend = (std::min)(atMost, available);

  RegisterId nrRegs = getPlanNode()->getRegisterPlan()->nrRegs[getPlanNode()->getDepth()];

  std::unique_ptr<AqlItemBlock> res(requestBlock(toSend, nrRegs));
  // std::unique_ptr<AqlItemBlock> res(new AqlItemBlock(toSend, nrRegs));
  // automatically freed if we throw
  TRI_ASSERT(curRegs <= res->getNrRegs());
  
  // only copy 1st row of registers inherited from previous frame(s)
  inheritRegisters(cur, res.get(), _pos);

  for (size_t j = 0; j < toSend; j++) {
    if (j > 0) {
      // re-use already copied aqlvalues
      for (RegisterId i = 0; i < curRegs; i++) {
        res->setValue(j, i, res->getValueReference(0, i));
        // Note: if this throws, then all values will be deleted
        // properly since the first one is.
      }
    }
    if (usesVertexOutput()) {
      auto p = _paths[_posInPaths].toJson(nullptr, nullptr);
      Json vertices = p.get("vertices");
      size_t length = vertices.size();
      AqlValue v(new Json(vertices.at(length - 1)));
      res->setValue(j,
        _vertexReg,
        v.clone()
      );
    }
    if (usesEdgeOutput()) {
      auto p = _paths[_posInPaths].toJson(nullptr, nullptr);
      Json edges = p.get("edges");
      size_t length = edges.size();
      AqlValue v(new Json(edges.at(length - 1)));
      res->setValue(j,
        _edgeReg,
        v.clone()
      );
    }
    if (usesPathOutput()) {
      res->setValue(j,
        _pathReg,
        _paths[_posInPaths].clone()
      );
    }
    ++_posInPaths;
  }
  // Advance read position:
  if (_posInPaths >= _paths.size()) {
    // we have exhausted our local paths buffer
    // fetch more paths into our buffer
    if (! morePaths(atMost)) {
      // nothing more to read, re-initialize fetching of paths
      if (++_pos >= cur->size()) {
        _buffer.pop_front();  // does not throw
        // returnBlock(cur);
        delete cur;
        _pos = 0;
      } else {
        initializePaths(cur);
      }
    }
  }

  // Clear out registers no longer needed later:
  clearRegisters(res.get());
  return res.release();
}

size_t TraversalBlock::skipSome (size_t atLeast, size_t atMost) {
  // TODO: FIXME
  size_t skipped = 0;

  return skipped;
}

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|// --SECTION--\\|/// @\\}\\)"
// End:
