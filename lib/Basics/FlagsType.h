////////////////////////////////////////////////////////////////////////////////
/// @brief flags base class
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
/// @author Copyright 2014, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2012-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGODB_BASICS_FLAGS_TYPE_H
#define ARANGODB_BASICS_FLAGS_TYPE_H 1

#include "Basics/Common.h"

namespace triagens {
  namespace basics {

// -----------------------------------------------------------------------------
// --SECTION--                                                   class FlagsType
// -----------------------------------------------------------------------------

    template<typename T>
    class FlagsType {

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

      protected:

        FlagsType () 
          : _flags(0) {
        }

// -----------------------------------------------------------------------------
// --SECTION--                                                 protected methods
// -----------------------------------------------------------------------------

      protected:

////////////////////////////////////////////////////////////////////////////////
/// @brief test a flag
////////////////////////////////////////////////////////////////////////////////

        bool hasFlag (T flag) const {
          return (_flags & flag) != 0;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief set a flag
////////////////////////////////////////////////////////////////////////////////

        void setFlag (T flag) {
          _flags |= flag;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief set or clear a flag
////////////////////////////////////////////////////////////////////////////////

        void setFlag (T flag, bool value) {
          if (value) {
            _flags |= flag;
          }
          else {
            _flags &= ~flag;
          }
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief clear a flag
////////////////////////////////////////////////////////////////////////////////

        void clearFlag (T value) {
          _flags &= ~value;
        }

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief the integer storing the flags
////////////////////////////////////////////////////////////////////////////////
      
        T _flags;

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
