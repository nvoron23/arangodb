////////////////////////////////////////////////////////////////////////////////
/// @brief Aql, Index
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
/// @author not James
/// @author Copyright 2014, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2012-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGODB_AQL_INDEX_H
#define ARANGODB_AQL_INDEX_H 1

#include "Basics/Common.h"
#include "Basics/json.h"
#include "Basics/JsonHelper.h"
#include "Mvcc/HashIndex.h"
#include "Mvcc/Index.h"
#include "Mvcc/SkiplistIndex.h"
#include "Utils/Exception.h"

namespace triagens {
  namespace aql {

// -----------------------------------------------------------------------------
// --SECTION--                                                      struct Index
// -----------------------------------------------------------------------------

    struct Index {

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

      Index (Index const&) = delete;
      Index& operator= (Index const&) = delete;
      
      Index (triagens::mvcc::Index* index)
        : id(index->id()),
          type(index->type()),
          unique(false),
          sparse(false),
          fields(index->fields()),
          internals(index) {

        TRI_ASSERT(internals != nullptr);

        if (index->type() == triagens::mvcc::Index::TRI_IDX_TYPE_HASH_INDEX) {
          auto idx = static_cast<triagens::mvcc::HashIndex const*>(index);
          unique = idx->unique();
          sparse = idx->sparse();
        }
        else if (index->type() == triagens::mvcc::Index::TRI_IDX_TYPE_SKIPLIST_INDEX) {
          auto idx = static_cast<triagens::mvcc::SkiplistIndex const*>(index);
          unique = idx->unique();
          sparse = idx->sparse();
        }
      }
      
      Index (TRI_json_t const* json)
        : id(triagens::basics::StringUtils::uint64(triagens::basics::JsonHelper::checkAndGetStringValue(json, "id"))),
          type(triagens::mvcc::Index::type(triagens::basics::JsonHelper::checkAndGetStringValue(json, "type").c_str())),
          unique(triagens::basics::JsonHelper::checkAndGetBooleanValue(json, "unique")),
          sparse(triagens::basics::JsonHelper::getBooleanValue(json, "sparse", false)),
          fields(),
          internals(nullptr) {

        TRI_json_t const* f = TRI_LookupObjectJson(json, "fields");

        if (TRI_IsArrayJson(f)) {
          size_t const n = TRI_LengthArrayJson(f);
          fields.reserve(n);

          for (size_t i = 0; i < n; ++i) {
            TRI_json_t const* name = static_cast<TRI_json_t const*>(TRI_AtVector(&f->_value._objects, i));
            if (TRI_IsStringJson(name)) {
              fields.emplace_back(std::string(name->_value._string.data, name->_value._string.length - 1));
            }
          }
        }

        // it is the caller's responsibility to fill the data attribute with something sensible later!
      }
      
      ~Index() {
      }
  
  
      triagens::basics::Json toJson () const {
        triagens::basics::Json json(triagens::basics::Json::Object);

        json("type", triagens::basics::Json(triagens::mvcc::Index::type(type)))
            ("id", triagens::basics::Json(triagens::basics::StringUtils::itoa(id))) 
            ("unique", triagens::basics::Json(unique))
            ("sparse", triagens::basics::Json(sparse));

        if (hasSelectivityEstimate()) {
          json("selectivityEstimate", triagens::basics::Json(selectivityEstimate()));
        }

        triagens::basics::Json f(triagens::basics::Json::Array);
        for (auto& field : fields) {
          f.add(triagens::basics::Json(field));
        }

        json("fields", f);
        return json;
      }

      bool hasSelectivityEstimate () const {
        if (! hasInternals()) { 
          return false;
        }

        return getInternals()->hasSelectivity();
      }

      double selectivityEstimate () const {
        auto index = getInternals();

        TRI_ASSERT(index->hasSelectivity());

        return index->getSelectivity();
      }
      
      inline bool hasInternals () const {
        return (internals != nullptr);
      }

      triagens::mvcc::Index* getInternals () const {
        if (internals == nullptr) {
          THROW_ARANGO_EXCEPTION_MESSAGE(TRI_ERROR_INTERNAL, "accessing undefined index internals");
        }
        return internals; 
      }
      
      void setInternals (triagens::mvcc::Index* index) {
        TRI_ASSERT(internals == nullptr);
        internals = index;
      }

// -----------------------------------------------------------------------------
// --SECTION--                                                  public variables
// -----------------------------------------------------------------------------

      public:

        TRI_idx_iid_t const                       id;
        triagens::mvcc::Index::IndexType const    type;
        bool                                      unique;
        bool                                      sparse;
        std::vector<std::string>                  fields;

      private:

        triagens::mvcc::Index*     internals;

    };

  }
}

#endif

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
