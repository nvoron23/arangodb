////////////////////////////////////////////////////////////////////////////////
/// @brief MVCC collection name resolver guard
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

#include "ScopedResolver.h"
#include "Mvcc/TransactionStackAccessor.h"
#include "Utils/CollectionNameResolver.h"
#include "VocBase/vocbase.h"

using namespace triagens::mvcc;

// -----------------------------------------------------------------------------
// --SECTION--                                              class ScopedResolver
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief join an existing transaction in an outer scope or a new transaction, 
/// which will be automatically freed when the scope is left
////////////////////////////////////////////////////////////////////////////////

ScopedResolver::ScopedResolver (TRI_vocbase_t* vocbase)
  : _resolver(nullptr),
    _ownsResolver(false) {
  
  // look on the stack for another transaction first
  TransactionStackAccessor accessor;
  auto transaction = accessor.peek();

  if (transaction != nullptr) {
    // found one, now check its resolver
    _resolver = transaction->resolver();
  }

  if (_resolver == nullptr) {
    // other transaction does not have a resolver. now create our own
    _resolver = new triagens::arango::CollectionNameResolver(vocbase);
    _ownsResolver = true;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the resolver if owned
////////////////////////////////////////////////////////////////////////////////

ScopedResolver::~ScopedResolver () {
  if (_ownsResolver) {
    // it was our own resolver. now delete it
    TRI_ASSERT(_resolver != nullptr);
    delete _resolver;
  }
}

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
