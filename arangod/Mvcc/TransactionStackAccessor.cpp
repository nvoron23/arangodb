////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC transactions stack
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

#include "TransactionStackAccessor.h"

using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                    class TransactionStackAccessor
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                            thread local variables
// -----------------------------------------------------------------------------

thread_local std::vector<Transaction*> TransactionStackAccessor::_threadTransactions;

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief create a stack accessor instance
////////////////////////////////////////////////////////////////////////////////

TransactionStackAccessor::TransactionStackAccessor () {
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the stack accessor
////////////////////////////////////////////////////////////////////////////////

TransactionStackAccessor::~TransactionStackAccessor () {
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
