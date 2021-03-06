/*global AQL_QUERY_CACHE_PROPERTIES, AQL_QUERY_CACHE_INVALIDATE */

////////////////////////////////////////////////////////////////////////////////
/// @brief AQL query cache management
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2012 triagens GmbH, Cologne, Germany
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
/// @author Jan Steemann
/// @author Copyright 2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

// -----------------------------------------------------------------------------
// --SECTION--                                   module "org/arangodb/aql/cache"
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief invalidates the query cache
////////////////////////////////////////////////////////////////////////////////

exports.clear = function () {
  'use strict';

  AQL_QUERY_CACHE_INVALIDATE();
};

////////////////////////////////////////////////////////////////////////////////
/// @brief fetches or sets the properties of the query cache
////////////////////////////////////////////////////////////////////////////////

exports.properties = function (properties) {
  'use strict';
 
  if (properties !== undefined) {
    return AQL_QUERY_CACHE_PROPERTIES(properties);
  }
  return AQL_QUERY_CACHE_PROPERTIES();
};

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// @addtogroup\\|// --SECTION--\\|/// @page\\|/// @}\\|/\\*jslint"
// End:

