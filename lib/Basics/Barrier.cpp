////////////////////////////////////////////////////////////////////////////////
/// @brief barrier for synchronization
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
/// @author Copyright 2013-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#include "Barrier.h"
#include "Basics/ConditionLocker.h"

using namespace triagens::basics;

// -----------------------------------------------------------------------------
// --SECTION--                                                           Barrier
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create a barrier for the specified number of waiters
////////////////////////////////////////////////////////////////////////////////

Barrier::Barrier (size_t size)
  : _condition(),
    _missing(size) {
 
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the barrier. this will call synchronize() to ensure all tasks
/// are joined
////////////////////////////////////////////////////////////////////////////////

Barrier::~Barrier () {
  synchronize();
}

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief join a single task. reduces the number of waiting tasks and wakes
/// up the barrier's synchronize() routine
////////////////////////////////////////////////////////////////////////////////

void Barrier::join () {
  {
    CONDITION_LOCKER(guard, _condition);
    TRI_ASSERT(_missing > 0);
    --_missing;
  }

  _condition.signal();
}

////////////////////////////////////////////////////////////////////////////////
/// @brief wait for all tasks to join
////////////////////////////////////////////////////////////////////////////////

void Barrier::synchronize () {
  while (true) {
    CONDITION_LOCKER(guard, _condition);

    if (_missing == 0) {
      break;
    }

    guard.wait(100000);
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
