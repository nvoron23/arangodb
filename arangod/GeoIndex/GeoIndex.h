////////////////////////////////////////////////////////////////////////////////
/// @brief geo index
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
/// @author R. A. Parker
/// @author Copyright 2014, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2011-2012, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

/* GeoIndex.h - header file for GeoIndex algorithms  */
/* Version 2.1   8.1.2012   R. A. Parker             */

#ifndef ARANGODB_GEO_INDEX_GEO_INDEX_H
#define ARANGODB_GEO_INDEX_GEO_INDEX_H 1

#include "Basics/Common.h"

namespace triagens {
  namespace mvcc {
    class Transaction;
  }
}

/* If this #define is there, then the INDEXDUMP and */
/* INDEXVALID functions are also available.  These  */
/* are not needed for normal production versions    */
/* the INDEXDUMP function also prints the data,     */
/* assumed to be a character string, if DEBUG is    */
/* set to 2.                                        */
#define TRI_GEO_DEBUG 1

typedef struct {
  double latitude;
  double longitude;
  void* data;
}
GeoCoordinate;

typedef struct {
  size_t length;
  GeoCoordinate* coordinates;
  double * distances;
}
GeoCoordinates;

typedef char GeoIndexApiType;   /* to keep the structure private  */


size_t GeoIndex_MemoryUsage (GeoIndexApiType*);

GeoIndexApiType* GeoIndex_new ();

void GeoIndex_free (GeoIndexApiType* gi);

int GeoIndex_insert (GeoIndexApiType* gi, 
                     GeoCoordinate* c);

int GeoIndex_remove (GeoIndexApiType* gi, 
                     GeoCoordinate* c);

double GeoIndex_distance (GeoCoordinate*, 
                          GeoCoordinate*);

// this version is used during tests
GeoCoordinates* GeoIndex_PointsWithinRadius (GeoIndexApiType*,
                                             GeoCoordinate*, 
                                             double);

GeoCoordinates* GeoIndex_PointsWithinRadius (triagens::mvcc::Transaction*,
                                             GeoIndexApiType*,
                                             GeoCoordinate*, 
                                             double); 

// this version is used during tests
GeoCoordinates* GeoIndex_NearestCountPoints (GeoIndexApiType*,
                                             GeoCoordinate*, 
                                             int);

GeoCoordinates* GeoIndex_NearestCountPoints (triagens::mvcc::Transaction*,
                                             GeoIndexApiType*,
                                             GeoCoordinate*, 
                                             int); 

void GeoIndex_CoordinatesFree (GeoCoordinates* clist);

#ifdef TRI_GEO_DEBUG
void GeoIndex_INDEXDUMP (GeoIndexApiType* gi, 
                         FILE* f);

int  GeoIndex_INDEXVALID (GeoIndexApiType* gi);
#endif

#endif

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
