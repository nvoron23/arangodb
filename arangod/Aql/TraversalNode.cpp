/// @brief Implementation of Traversal Execution Node
///
/// @file arangod/Aql/TraversalNode.cpp
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
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Michael Hackstein
/// @author Copyright 2015, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "Aql/TraversalNode.h"
#include "Aql/ExecutionPlan.h"

using namespace std;
using namespace triagens::basics;
using namespace triagens::aql;

TraversalNode::TraversalNode (ExecutionPlan* plan,
               size_t id,
               TRI_vocbase_t* vocbase, 
               AstNode const* direction,
               AstNode const* start,
               AstNode const* graph)
  : ExecutionNode(plan, id), 
    _start(start),
    _vocbase(vocbase), 
    _direction(direction),
    _graph(graph),
    _steps(nullptr),
    _resolver(new arango::CollectionNameResolver(vocbase))
{

  TRI_ASSERT(_vocbase != nullptr);
  TRI_ASSERT(_direction != nullptr);
  TRI_ASSERT(_start != nullptr);
  TRI_ASSERT(_resolver != nullptr);
  _vertexOutVariable = nullptr;
  _edgeOutVariable = nullptr;
  _pathOutVariable = nullptr;
  if (_graph->type == NODE_TYPE_COLLECTION_LIST) {
    // List of edge collection names
    for (size_t i = 0; i <  _graph->numMembers(); ++i) {
      auto eColName = _graph->getMember(i)->getStringValue();
      auto edgeStruct = _resolver->getCollectionStruct(eColName);
      if (edgeStruct->_type != TRI_COL_TYPE_EDGE) {
        THROW_ARANGO_EXCEPTION(TRI_ERROR_ARANGO_COLLECTION_TYPE_INVALID);
      }
      _edgeCids.push_back(edgeStruct->_cid);
    }
  } else {
    if (_edgeCids.size() == 0) {
      if (_graph->isStringValue()) {
        auto graph = triagens::arango::GraphFactory::factory()->byName(
          _vocbase,
          _graph->getStringValue()
        );
        auto eColls = graph.edgeCollections();
        for (const auto& n: eColls) {
          TRI_voc_cid_t cid = _resolver->getCollectionId(n);
          _edgeCids.push_back(cid);
        }
      }
    }
  }
  if (_start->type == NODE_TYPE_REFERENCE) {
    _inVariable = static_cast<Variable*>(_start->getData());
  } else {
    _inVariable = nullptr;
  }
}



TraversalNode::TraversalNode (ExecutionPlan* plan,
                              triagens::basics::Json const& base)
  : ExecutionNode(plan, base),
    _start(nullptr),
    _vocbase(plan->getAst()->query()->vocbase()),
    // _outVariable(varFromJson(plan->getAst(), base, "outVariable")),
    _direction(nullptr),
    _graph(nullptr),
    _steps(nullptr) { // TODO: FIXME
  TRI_ASSERT(false);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief toJson, for TraversalNode
////////////////////////////////////////////////////////////////////////////////

void TraversalNode::toJsonHelper (triagens::basics::Json& nodes,
                                  TRI_memory_zone_t* zone,
                                  bool verbose) const {
  triagens::basics::Json json(ExecutionNode::toJsonHelperGeneric(nodes, zone, verbose));  // call base class method

  if (json.isEmpty()) {
    return;
  }

  // Now put info about vocbase and cid in there
  json("database", triagens::basics::Json(_vocbase->_name));
      // ("outVariable", _outVariable->toJson());

  // TODO: FIXME

  // And add it:
  nodes(json);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief clone ExecutionNode recursively
////////////////////////////////////////////////////////////////////////////////

ExecutionNode* TraversalNode::clone (ExecutionPlan* plan,
                                     bool withDependencies,
                                     bool withProperties) const {
  auto c = new TraversalNode(plan, _id, _vocbase, _direction, _start, _graph);

  if (usesVertexOutVariable()) {
    auto vertexOutVariable = _vertexOutVariable;
    if (withProperties) {
      vertexOutVariable = plan->getAst()->variables()->createVariable(vertexOutVariable);
    }
    TRI_ASSERT(vertexOutVariable != nullptr);
    c->setVertexOutput(vertexOutVariable);
  }

  if (usesEdgeOutVariable()) {
    auto edgeOutVariable = _edgeOutVariable;
    if (withProperties) {
      edgeOutVariable = plan->getAst()->variables()->createVariable(edgeOutVariable);
    }
    TRI_ASSERT(edgeOutVariable != nullptr);
    c->setEdgeOutput(edgeOutVariable);
  }

  if (usesPathOutVariable()) {
    auto pathOutVariable = _pathOutVariable;
    if (withProperties) {
      pathOutVariable = plan->getAst()->variables()->createVariable(pathOutVariable);
    }
    TRI_ASSERT(pathOutVariable != nullptr);
    c->setPathOutput(pathOutVariable);
  }

  cloneHelper(c, plan, withDependencies, withProperties);

  return static_cast<ExecutionNode*>(c);
}

////////////////////////////////////////////////////////////////////////////////
/// @brief the cost of a traversal node
////////////////////////////////////////////////////////////////////////////////
        
double TraversalNode::estimateCost (size_t& nrItems) const { 
  size_t incoming;
  double depCost = _dependencies.at(0)->getCost(incoming);
  size_t count = 1000; // TODO: FIXME
  nrItems = incoming * count;
  return depCost + nrItems;
}

void TraversalNode::fillTraversalOptions (basics::traverser::TraverserOptions& opts) const {
  // This check is only necessary during transaction old => new
  TRI_ASSERT(_direction->type == NODE_TYPE_DIRECTION);
  TRI_ASSERT(_direction->numMembers() == 2);
  // Member 0 is the direction. Already the correct Integer.
  // Is not inserted by user but by enum.
  auto dir = _direction->getMember(0);
  auto steps = _direction->getMember(1);

  TRI_ASSERT(dir->isNumericValue());
  auto dirNum = dir->getIntValue();
  switch (dirNum) {
    case 0:
      opts.direction = TRI_EDGE_ANY;
      break;
    case 1:
      opts.direction = TRI_EDGE_IN;
      break;
    case 2:
      opts.direction = TRI_EDGE_OUT;
      break;
    default:
      TRI_ASSERT(false);
      break;
  }

  if (steps->isNumericValue()) {
    TRI_ASSERT(steps->isNumericValue());
    opts.minDepth = steps->getIntValue();
    opts.maxDepth = steps->getIntValue();
  } else if (steps->type == NODE_TYPE_RANGE) {
    // Range depth
    auto lhs = steps->getMember(0);
    auto rhs = steps->getMember(1);
    if (lhs->isNumericValue()) {
      // Range is left-closed
      opts.minDepth = lhs->getIntValue();
    }
    if (rhs->isNumericValue()) {
      // Range is right-closed
      opts.maxDepth = rhs->getIntValue();
    }
  } else {
    // TODO FIXME
    TRI_ASSERT(false);
  }
}




// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|// --SECTION--\\|/// @\\}\\)"
// End:
