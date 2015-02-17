////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC geo index
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

#ifndef ARANGODB_MVCC_GEO_INDEX_H
#define ARANGODB_MVCC_GEO_INDEX_H 1

#include "Basics/Common.h"
#include "Basics/JsonHelper.h"
#include "GeoIndex/GeoIndex.h"
#include "Mvcc/Index.h"
#include "ShapedJson/shaped-json.h"
#include "VocBase/index.h"

struct TRI_doc_mptr_t;
struct TRI_document_collection_t;
struct TRI_json_t;
struct TRI_transaction_collection_s;

namespace triagens {
  namespace mvcc {

// -----------------------------------------------------------------------------
// --SECTION--                                                    class GeoIndex
// -----------------------------------------------------------------------------

    class GeoIndex2 : public Index {

      public:

        GeoIndex2 (TRI_idx_iid_t id,
                   struct TRI_document_collection_t*,
                   std::vector<std::string> const&,
                   std::vector<TRI_shape_pid_t> const&,
                   bool unique,
                   bool ignoreNull,
                   bool geoJson);
        
        GeoIndex2 (TRI_idx_iid_t id,
                   struct TRI_document_collection_t*,
                   std::vector<std::string> const&,
                   std::vector<TRI_shape_pid_t> const&,
                   bool unique,
                   bool ignoreNull);

        ~GeoIndex2 ();

      public:

        std::vector<std::pair<TRI_doc_mptr_t*, double>>* near (Transaction*,
                                                               double,
                                                               double,
                                                               size_t);
        
        std::vector<std::pair<TRI_doc_mptr_t*, double>>* within (Transaction*,
                                                                 double,
                                                                 double,
                                                                 double);
        
        void insert (struct TRI_doc_mptr_t*) override final;
  
        void insert (TransactionCollection*, 
                     Transaction*,
                     struct TRI_doc_mptr_t*) override final;

        TRI_doc_mptr_t* remove (TransactionCollection*,
                                Transaction*,
                                std::string const&,
                                struct TRI_doc_mptr_t*) override final;

        void forget (TransactionCollection*,
                     Transaction*,
                     struct TRI_doc_mptr_t*) override final;

        void preCommit (TransactionCollection*,
                        Transaction*) override final;

        bool hasSelectivity () const override final {
          return false;
        }

        double getSelectivity () const override final {
          return -1.0;
        }

        size_t memory () override final;
        triagens::basics::Json toJson (TRI_memory_zone_t*) const override final;

        TRI_idx_type_e type () const override final {
          if (_variant == INDEX_GEO_COMBINED_LAT_LON || 
              _variant == INDEX_GEO_COMBINED_LON_LAT) {
            return TRI_IDX_TYPE_GEO1_INDEX;
          }
          return TRI_IDX_TYPE_GEO2_INDEX;
        }
        
        std::string typeName () const override final {
          if (_variant == INDEX_GEO_COMBINED_LAT_LON || 
              _variant == INDEX_GEO_COMBINED_LON_LAT) {
            return "geo1";
          }
          return "geo2";
        }

      private:

        bool extractDoubleArray (struct TRI_shaper_s*,
                                 struct TRI_shaped_json_s const*,
                                 TRI_shape_pid_t,
                                 double*,
                                 bool*);

        bool extractDoubleList (struct TRI_shaper_s*,
                                struct TRI_shaped_json_s const*,
                                TRI_shape_pid_t,
                                double*,
                                double*,
                                bool*);

      private:
 
        std::vector<TRI_shape_pid_t> const _paths; 
        GeoIndex* _geoIndex;
        TRI_index_geo_variant_e const _variant;

        TRI_shape_pid_t _location;
        TRI_shape_pid_t _latitude;
        TRI_shape_pid_t _longitude;

        bool _geoJson;
        bool _unique;
        bool _ignoreNull;
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
