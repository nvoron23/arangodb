////////////////////////////////////////////////////////////////////////////////
/// @brief compactor
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
/// @author Dr. Frank Celler
/// @author Copyright 2014, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGODB_VOC_BASE_COMPACTOR_H
#define ARANGODB_VOC_BASE_COMPACTOR_H 1

#include "Basics/Common.h"

#include "VocBase/voc-types.h"

struct TRI_vocbase_t;

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief initialize the compaction blockers structure
////////////////////////////////////////////////////////////////////////////////

int TRI_InitCompactorVocBase (TRI_vocbase_t*);

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the compaction blockers structure
////////////////////////////////////////////////////////////////////////////////

void TRI_DestroyCompactorVocBase (TRI_vocbase_t*);

////////////////////////////////////////////////////////////////////////////////
/// @brief remove data of expired compaction blockers
////////////////////////////////////////////////////////////////////////////////

bool TRI_CleanupCompactorVocBase (TRI_vocbase_t*);

////////////////////////////////////////////////////////////////////////////////
/// @brief insert a compaction blocker
////////////////////////////////////////////////////////////////////////////////

int TRI_InsertBlockerCompactorVocBase (TRI_vocbase_t*,
                                       double,
                                       TRI_voc_tick_t*);

////////////////////////////////////////////////////////////////////////////////
/// @brief touch an existing compaction blocker
////////////////////////////////////////////////////////////////////////////////

int TRI_TouchBlockerCompactorVocBase (TRI_vocbase_t*,
                                      TRI_voc_tick_t,
                                      double);

////////////////////////////////////////////////////////////////////////////////
/// @brief remove an existing compaction blocker
////////////////////////////////////////////////////////////////////////////////

int TRI_RemoveBlockerCompactorVocBase (TRI_vocbase_t*,
                                       TRI_voc_tick_t);

////////////////////////////////////////////////////////////////////////////////
/// @brief atomically check-and-lock the compactor
/// if the function returns true, then a write-lock on the compactor was
/// acquired, which must eventually be freed by the caller
////////////////////////////////////////////////////////////////////////////////

bool TRI_CheckAndLockCompactorVocBase (TRI_vocbase_t*);

////////////////////////////////////////////////////////////////////////////////
/// @brief unlock the compactor
////////////////////////////////////////////////////////////////////////////////

void TRI_UnlockCompactorVocBase (TRI_vocbase_t*);

////////////////////////////////////////////////////////////////////////////////
/// @brief compactor event loop
////////////////////////////////////////////////////////////////////////////////

void TRI_CompactorVocBase (void*);

#endif

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
