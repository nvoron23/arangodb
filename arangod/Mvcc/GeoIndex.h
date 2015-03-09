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
#include "Basics/ReadWriteLock.h"
#include "GeoIndex/GeoIndex.h"
#include "Mvcc/Index.h"
#include "ShapedJson/shaped-json.h"

struct TRI_document_collection_t;

namespace triagens {
  namespace mvcc {

// -----------------------------------------------------------------------------
// --SECTION--                                                    class GeoIndex
// -----------------------------------------------------------------------------

    class GeoIndex : public Index {

// -----------------------------------------------------------------------------
// --SECTION--                                                     private types
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief geo index variants
////////////////////////////////////////////////////////////////////////////////

      enum IndexVariant {
        INDEX_GEO_NONE = 0,
        INDEX_GEO_INDIVIDUAL_LAT_LON,
        INDEX_GEO_COMBINED_LAT_LON,
        INDEX_GEO_COMBINED_LON_LAT
      };

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

      public:

        GeoIndex (TRI_idx_iid_t id,
                  struct TRI_document_collection_t*,
                  std::vector<std::string> const&,
                  std::vector<TRI_shape_pid_t> const&,
                  bool geoJson);
        
        GeoIndex (TRI_idx_iid_t id,
                  struct TRI_document_collection_t*,
                  std::vector<std::string> const&,
                  std::vector<TRI_shape_pid_t> const&);

        ~GeoIndex ();

// -----------------------------------------------------------------------------
// --SECTION--                            public methods, inherited from Index.h
// -----------------------------------------------------------------------------

      public:

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

        Index::IndexType type () const override final {
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
        
        void clickLock () override final {
          _lock.writeLock();
          _lock.writeUnlock();
        }

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

      public:
        
        std::vector<std::pair<TRI_doc_mptr_t*, double>>* near (Transaction*,
                                                               double,
                                                               double,
                                                               size_t);
        
        std::vector<std::pair<TRI_doc_mptr_t*, double>>* within (Transaction*,
                                                                 double,
                                                                 double,
                                                                 double);

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------
        
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

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief the underlying collection
////////////////////////////////////////////////////////////////////////////////
  
        struct TRI_document_collection_t* _collection;

////////////////////////////////////////////////////////////////////////////////
/// @brief the index R/W lock
////////////////////////////////////////////////////////////////////////////////
        
        triagens::basics::ReadWriteLock   _lock;

////////////////////////////////////////////////////////////////////////////////
/// @brief the attribute paths
////////////////////////////////////////////////////////////////////////////////
 
        std::vector<TRI_shape_pid_t> const _paths; 

////////////////////////////////////////////////////////////////////////////////
/// @brief the geo index
////////////////////////////////////////////////////////////////////////////////

        GeoIndexApiType* _geoIndex;

////////////////////////////////////////////////////////////////////////////////
/// @brief the geo index variant (geo1 or geo2)
////////////////////////////////////////////////////////////////////////////////

        IndexVariant const _variant;

////////////////////////////////////////////////////////////////////////////////
/// @brief attribute paths
////////////////////////////////////////////////////////////////////////////////

        TRI_shape_pid_t _location;
        TRI_shape_pid_t _latitude;
        TRI_shape_pid_t _longitude;

////////////////////////////////////////////////////////////////////////////////
/// @brief whether the index is a geoJson index (latitude / longitude reversed)
////////////////////////////////////////////////////////////////////////////////

        bool _geoJson;

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
