////////////////////////////////////////////////////////////////////////////////
/// @brief fundamental types for the optimisation and execution of AQL
///
/// @file arangod/Aql/Types.h
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
/// @author Max Neunhoeffer
/// @author Copyright 2014, triagens GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGODB_AQL_TYPES_H
#define ARANGODB_AQL_TYPES_H 1

#include <functional>

#include <Basics/JsonHelper.h>
#include <BasicsC/json-utilities.h>

#include "Aql/AstNode.h"
#include "Aql/Variable.h"
#include "V8/v8-conv.h"
#include "VocBase/document-collection.h"
#include "VocBase/voc-shaper.h"
#include "V8Server/v8-vocbase.h"
#include "V8Server/v8-wrapshapedjson.h"
#include "Utils/V8TransactionContext.h"
#include "Utils/AqlTransaction.h"

namespace triagens {
  namespace aql {

// -----------------------------------------------------------------------------
// --SECTION--                                                            AqlDoc
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief type for register numbers/ids
////////////////////////////////////////////////////////////////////////////////

    typedef unsigned int RegisterId;

////////////////////////////////////////////////////////////////////////////////
/// @brief forward declaration for blocks of items
////////////////////////////////////////////////////////////////////////////////

    class AqlItemBlock;

////////////////////////////////////////////////////////////////////////////////
/// @brief a struct to hold a value, registers hole AqlValue* during the 
/// execution
////////////////////////////////////////////////////////////////////////////////

    struct AqlValue {

////////////////////////////////////////////////////////////////////////////////
/// @brief AqlValueType, indicates what sort of value we have
////////////////////////////////////////////////////////////////////////////////

      enum AqlValueType {
        EMPTY,     // contains no data
        JSON,      // Json*
        SHAPED,    // TRI_df_marker_t*
        DOCVEC,    // a vector of blocks of results coming from a subquery
        RANGE      // a pointer to a range remembering lower and upper bound
      };

////////////////////////////////////////////////////////////////////////////////
/// @brief Range, to hold a range compactly
////////////////////////////////////////////////////////////////////////////////

      struct Range {
        int64_t const _low;
        int64_t const _high;
        Range (int64_t low, int64_t high) : _low(low), _high(high) {}
      };

////////////////////////////////////////////////////////////////////////////////
/// @brief the actual data
////////////////////////////////////////////////////////////////////////////////

      union {
        triagens::basics::Json*     _json;
        TRI_df_marker_t const*      _marker;
        std::vector<AqlItemBlock*>* _vector;
        Range const*                _range;
      };
      
////////////////////////////////////////////////////////////////////////////////
/// @brief _type, the type of value
////////////////////////////////////////////////////////////////////////////////

      AqlValueType _type;

////////////////////////////////////////////////////////////////////////////////
/// @brief constructors for the various value types, note that they all take
/// ownership of the corresponding pointers
////////////////////////////////////////////////////////////////////////////////

      AqlValue () : _json(nullptr), _type(EMPTY) {
      }

      AqlValue (triagens::basics::Json* json)
        : _json(json), _type(JSON) {
      }
      
      AqlValue (TRI_df_marker_t const* marker)
        : _marker(marker), _type(SHAPED) {
      }
      
      AqlValue (std::vector<AqlItemBlock*>* vector)
        : _vector(vector), _type(DOCVEC) {
      }

      AqlValue (int64_t low, int64_t high) 
        : _type(RANGE) {
        _range = new Range(low, high);
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief destructor, doing nothing automatically!
////////////////////////////////////////////////////////////////////////////////

      ~AqlValue () {
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy, explicit destruction, only when needed
////////////////////////////////////////////////////////////////////////////////

      void destroy ();

////////////////////////////////////////////////////////////////////////////////
/// @brief erase, this does not free the stuff in the AqlValue, it only
/// erases the pointers and makes the AqlValue structure EMPTY, this
/// is used when the AqlValue is stolen and stored in another object
////////////////////////////////////////////////////////////////////////////////

      void erase () {
        _type = EMPTY;
        _json = nullptr;
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief clone for recursive copying
////////////////////////////////////////////////////////////////////////////////

      AqlValue clone () const;

////////////////////////////////////////////////////////////////////////////////
/// @brief construct a V8 value as input for the expression execution in V8
////////////////////////////////////////////////////////////////////////////////

      v8::Handle<v8::Value> toV8 (AQL_TRANSACTION_V8* trx, 
                                  TRI_document_collection_t const*) const;

////////////////////////////////////////////////////////////////////////////////
/// @brief toString method
////////////////////////////////////////////////////////////////////////////////

      std::string toString (TRI_document_collection_t const*) const;
      
////////////////////////////////////////////////////////////////////////////////
/// @brief toJson method
////////////////////////////////////////////////////////////////////////////////
      
      triagens::basics::Json toJson (TRI_document_collection_t const*) const;

////////////////////////////////////////////////////////////////////////////////
/// @brief a quick method to decide whether a value is empty
////////////////////////////////////////////////////////////////////////////////

      bool isEmpty () {
        return _type == EMPTY;
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief a quick method to decide whether a value is true
////////////////////////////////////////////////////////////////////////////////

      bool isTrue () {
        if (_type != JSON) {
          return false;
        }
        TRI_json_t* json = _json->json();
        if (TRI_IsBooleanJson(json) && json->_value._boolean) {
          return true;
        }
        else if (TRI_IsNumberJson(json) && json->_value._number != 0.0) {
          return true;
        }
        else if (TRI_IsStringJson(json) && json->_value._string.length != 0) {
          return true;
        }
        else {
          return false;
        }
      }
    };

////////////////////////////////////////////////////////////////////////////////
/// @brief 3-way comparison for AqlValue objects
////////////////////////////////////////////////////////////////////////////////

int CompareAqlValues ( AqlValue const& left,  
                       TRI_document_collection_t const* leftcoll,
                       AqlValue const& right, 
                       TRI_document_collection_t const* rightcoll ) {
        if (_type != right._type) {
          if (_type == EMPTY) {
            return -1;
          }
          if (right._type == EMPTY) {
            return 1;
          }
          if (_type == JSON && right._type == SHAPED) {
            triagens::basics::Json rjson = right.toJson(rightcoll);
            return TRI_CompareValuesJson(_json->json(), rjson.json());
          }
          if (_type == SHAPED && right._type == JSON) {
            triagens::basics::Json ljson = toJson(leftcoll);
            return TRI_CompareValuesJson(ljson.json(), right._json->json());
          }
          // No other comparisons are defined
          TRI_ASSERT(false);
        }
        switch (_type) {
          case EMPTY: {
            return 0;
          }
          case JSON: {
            return TRI_CompareValuesJson(_json->json(), right._json->json());
          }
          case SHAPED: {
            TRI_shaped_json_t l;
            TRI_shaped_json_t r;
            TRI_EXTRACT_SHAPED_JSON_MARKER(l, _marker);
            TRI_EXTRACT_SHAPED_JSON_MARKER(r, right._marker);
                      
            return TRI_CompareShapeTypes(nullptr, nullptr, &l, leftcoll->getShaper(), 
                                         nullptr, nullptr, &r, rightcoll->getShaper());
          }
          case DOCVEC: { 
            // use lexicographic ordering of AqlValues regardless of block,
            // DOCVECs have a single register coming from ReturnNode.
            size_t lblock = 0;
            size_t litem = 0;
            size_t rblock = 0;
            size_t ritem = 0;
            
            while( lblock < _vector->size() && rblock < right._vector->size() ){
              AqlValue lval = ((*_vector->at(lblock)).getValue(litem, 0);
              AqlValue rval = right._vector->at(rblock).getValue(ritem, 0);
              cmp = CompareAqlValues(lval, lval.getDocumentCollection(0)
                                     rval, rval.getDocumentCollection(0));
              if(cmp != 0){
                return cmp;
              }
              if(litem == this[lblock].size()-1){
                litem = 0;
                lblock++;
              }
              if(ritem == right[rblock].size()-1){
                ritem = 0;
                rblock++;
              }
            }

            if(lblock == _vector->size() && rblock == right._vector->size()){
              return 0;
            }

            return (lblock < _vector->size()?-1:1);
          }
          case RANGE: {
            if(_range->_low < right._range->_low){
              return -1;
            } 
            if (_range->_low > right._range->_low){
              return 1;
            } 
            if (_range->_high < _range->_high) {
              return -1;
            } 
            if (_range->_high > _range->_high) {
              return 1;
            }
            return 0;
          }
          default: {
            TRI_ASSERT(false);
            return 0;
          }
        }


}


////////////////////////////////////////////////////////////////////////////////
/// @brief hash function for AqlValue objects
////////////////////////////////////////////////////////////////////////////////

namespace std {

template<> struct hash<triagens::aql::AqlValue> {
  size_t operator () (triagens::aql::AqlValue const& x) const {
    std::hash<uint32_t> intHash;
    std::hash<void const*> ptrHash;
    size_t res = intHash(static_cast<uint32_t>(x._type));
    switch (x._type) {
      case triagens::aql::AqlValue::EMPTY: {
        return res;
      }
      case triagens::aql::AqlValue::JSON: {
        return res ^ ptrHash(x._json);
      }
      case triagens::aql::AqlValue::SHAPED: {
        return res ^ ptrHash(x._marker);
      }
      case triagens::aql::AqlValue::DOCVEC: {
        return res ^ ptrHash(x._vector);
      }
      case triagens::aql::AqlValue::RANGE: {
        return res ^ ptrHash(x._range);
      }
      default: {
        TRI_ASSERT(false);
        return 0;
      }
    }
  }
};

template<> struct equal_to<triagens::aql::AqlValue> {
  bool operator () (triagens::aql::AqlValue const& a,
                    triagens::aql::AqlValue const& b) const {
    if (a._type != b._type) {
      return false;
    }
    switch (a._type) {
      case triagens::aql::AqlValue::JSON: {
        return a._json == b._json;
      }
      case triagens::aql::AqlValue::SHAPED: {
        return a._marker == b._marker;
      }
      case triagens::aql::AqlValue::DOCVEC: {
        return a._vector == b._vector;
      }
      case triagens::aql::AqlValue::RANGE: {
        return a._range == b._range;
      }
      default: {
        TRI_ASSERT(false);
        return true;
      }
    }
  }
};

}

namespace triagens {
  namespace aql {

// -----------------------------------------------------------------------------
// --SECTION--                                                      AqlItemBlock
// -----------------------------------------------------------------------------

    class AqlItemBlock {

        std::vector<AqlValue> _data;
        std::vector<TRI_document_collection_t const*> _docColls;
        size_t     _nrItems;
        RegisterId _nrRegs;

      public:

        AqlItemBlock (size_t nrItems, RegisterId nrRegs)
          : _nrItems(nrItems), _nrRegs(nrRegs) {
          if (nrItems > 0 && nrRegs > 0) {
            _data.reserve(nrItems * nrRegs);
            for (size_t i = 0; i < nrItems * nrRegs; ++i) {
              _data.emplace_back();
            }
          }
          if (nrRegs > 0) {
            _docColls.reserve(nrRegs);
            for (size_t i = 0; i < nrRegs; ++i) {
              _docColls.push_back(nullptr);
            }
          }
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief destructor
////////////////////////////////////////////////////////////////////////////////

        ~AqlItemBlock () {
          std::unordered_set<AqlValue> cache;
          for (size_t i = 0; i < _nrItems * _nrRegs; i++) {
            if (! _data[i].isEmpty()) {
              auto it = cache.find(_data[i]);
              if (it == cache.end()) {
                cache.insert(_data[i]);
                _data[i].destroy();
              }
            }
          }
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief getValue, get the value of a register
////////////////////////////////////////////////////////////////////////////////

      AqlValue getValue (size_t index, RegisterId varNr) const {
        return _data[index * _nrRegs + varNr];
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief setValue, set the current value of a register
////////////////////////////////////////////////////////////////////////////////

      void setValue (size_t index, RegisterId varNr, AqlValue zeug) {
        TRI_ASSERT(_data[index * _nrRegs + varNr].isEmpty());
        _data[index * _nrRegs + varNr] = zeug;
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief eraseValue, erase the current value of a register not freeing it
/// this is used if the value is stolen and later released from elsewhere
////////////////////////////////////////////////////////////////////////////////

      void eraseValue (size_t index, RegisterId varNr) {
        _data[index * _nrRegs + varNr].erase();
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief getDocumentCollection
////////////////////////////////////////////////////////////////////////////////

      TRI_document_collection_t const* getDocumentCollection (RegisterId varNr) const {
        return _docColls[varNr];
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief setDocumentCollection, set the current value of a variable or attribute
////////////////////////////////////////////////////////////////////////////////

      void setDocumentCollection (RegisterId varNr, TRI_document_collection_t const* docColl) {
        _docColls[varNr] = docColl;
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief getter for _nrRegs
////////////////////////////////////////////////////////////////////////////////

      RegisterId getNrRegs () const {
        return _nrRegs;
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief getter for _nrItems
////////////////////////////////////////////////////////////////////////////////

      size_t size () const {
        return _nrItems;
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief getter for _data
////////////////////////////////////////////////////////////////////////////////

      vector<AqlValue>& getData () {
        return _data;
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief getter for _data
////////////////////////////////////////////////////////////////////////////////

      vector<TRI_document_collection_t const*>& getDocumentCollections () {
        return _docColls;
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief slice/clone
////////////////////////////////////////////////////////////////////////////////

      AqlItemBlock* slice (size_t from, size_t to) {
        TRI_ASSERT(from < to && to <= _nrItems);

        std::unordered_map<AqlValue, AqlValue> cache;
        auto res = new AqlItemBlock(to - from, _nrRegs);
        for (RegisterId col = 0; col < _nrRegs; col++) {
          res->_docColls[col] = _docColls[col];
        }
        for (size_t row = from; row < to; row++) {
          for (RegisterId col = 0; col < _nrRegs; col++) {
            AqlValue& a(_data[row * _nrRegs + col]);

            if (! a.isEmpty()) {
              auto it = cache.find(a);
              if (it == cache.end()) {
                AqlValue b = a.clone();
                res->_data[(row - from) * _nrRegs + col] = b;
                cache.insert(make_pair(a,b));
              }
              else {
                res->_data[(row - from) * _nrRegs + col] = it->second;
              }
            }
          }
        }
        return res;
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief slice/clone for a subset
////////////////////////////////////////////////////////////////////////////////

      AqlItemBlock* slice (vector<size_t>& chosen, size_t from, size_t to) {
        TRI_ASSERT(from < to && to <= chosen.size());

        std::unordered_map<AqlValue, AqlValue> cache;
        auto res = new AqlItemBlock(to - from, _nrRegs);
        for (RegisterId col = 0; col < _nrRegs; col++) {
          res->_docColls[col] = _docColls[col];
        }
        for (size_t row = from; row < to; row++) {
          for (RegisterId col = 0; col < _nrRegs; col++) {
            AqlValue& a(_data[chosen[row] * _nrRegs + col]);

            if (! a.isEmpty()) {
              auto it = cache.find(a);
              if (it == cache.end()) {
                AqlValue b = a.clone();
                res->_data[(row - from) * _nrRegs + col] = b;
                cache.insert(make_pair(a,b));
              }
              else {
                res->_data[(row - from) * _nrRegs + col] = it->second;
              }
            }
          }
        }
        return res;
      }

////////////////////////////////////////////////////////////////////////////////
/// @brief splice multiple blocks, note that the new block now owns all
/// AqlValue pointers in the old blocks, therefore, the latter are all
/// set to nullptr, just to be sure.
////////////////////////////////////////////////////////////////////////////////

      static AqlItemBlock* splice(std::vector<AqlItemBlock*>& blocks);

    };

  }  // namespace triagens::aql
}  // namespace triagens

#endif

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|// --SECTION--\\|/// @\\}\\)"
// End:

