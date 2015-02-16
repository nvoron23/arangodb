////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC collection statistics
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

#ifndef ARANGODB_MVCC_COLLECTION_STATS_H
#define ARANGODB_MVCC_COLLECTION_STATS_H 1

#include "Basics/Common.h"
#include "VocBase/voc-types.h"

namespace triagens {
  namespace mvcc {

// -----------------------------------------------------------------------------
// --SECTION--                                            struct CollectionStats
// -----------------------------------------------------------------------------

    struct CollectionStats {
      inline bool hasModifications () const {
        return (numInserted > 0 || numRemoved > 0);
      }

      void merge (CollectionStats const& other) {
        numInserted += other.numInserted;
        numRemoved  += other.numRemoved;

        updateRevision(other.revisionId);
        
        waitForSync |= other.waitForSync;
      }
      
      inline void updateRevision (TRI_voc_rid_t other) {
        if (other > revisionId) {
          revisionId = other;
        }
      }

      size_t numInserted        = 0;
      size_t numRemoved         = 0;
      TRI_voc_rid_t revisionId  = 0;

      bool   waitForSync        = false;
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
