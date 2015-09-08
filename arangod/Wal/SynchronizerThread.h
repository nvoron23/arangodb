////////////////////////////////////////////////////////////////////////////////
/// @brief Write-ahead log synchronizer thread
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
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGODB_WAL_SYNCHRONIZER_THREAD_H
#define ARANGODB_WAL_SYNCHRONIZER_THREAD_H 1

#include "Basics/Common.h"
#include "Basics/ConditionVariable.h"
#include "Basics/Thread.h"
#include "Wal/Logfile.h"
#include "Wal/SyncRegion.h"

namespace triagens {
  namespace wal {

    class LogfileManager;

// -----------------------------------------------------------------------------
// --SECTION--                                          class SynchronizerThread
// -----------------------------------------------------------------------------

    class SynchronizerThread : public basics::Thread {

////////////////////////////////////////////////////////////////////////////////
/// @brief SynchronizerThread
////////////////////////////////////////////////////////////////////////////////

      private:
        SynchronizerThread (SynchronizerThread const&) = delete;
        SynchronizerThread& operator= (SynchronizerThread const&) = delete;

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief create the synchronizer thread
////////////////////////////////////////////////////////////////////////////////

        SynchronizerThread (LogfileManager*,
                            uint64_t,
                            uint64_t);

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the synchronizer thread
////////////////////////////////////////////////////////////////////////////////

        ~SynchronizerThread ();

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief stops the synchronizer thread
////////////////////////////////////////////////////////////////////////////////

        void stop ();

////////////////////////////////////////////////////////////////////////////////
/// @brief signal that a sync is needed
////////////////////////////////////////////////////////////////////////////////

        void signalSync ();

// -----------------------------------------------------------------------------
// --SECTION--                                                    Thread methods
// -----------------------------------------------------------------------------

      protected:

////////////////////////////////////////////////////////////////////////////////
/// @brief main loop
////////////////////////////////////////////////////////////////////////////////

        void run ();

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief synchronize an unsynchronized region
////////////////////////////////////////////////////////////////////////////////

        int doSync (bool&);

////////////////////////////////////////////////////////////////////////////////
/// @brief get a logfile descriptor (it caches the descriptor for performance)
////////////////////////////////////////////////////////////////////////////////

        int getLogfileDescriptor (Logfile::IdType);

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief the logfile manager
////////////////////////////////////////////////////////////////////////////////

        LogfileManager* _logfileManager;

////////////////////////////////////////////////////////////////////////////////
/// @brief condition variable for the thread
////////////////////////////////////////////////////////////////////////////////

        basics::ConditionVariable _condition;

////////////////////////////////////////////////////////////////////////////////
/// @brief number of requests waiting
////////////////////////////////////////////////////////////////////////////////

        uint32_t _waiting;

////////////////////////////////////////////////////////////////////////////////
/// @brief stop flag
////////////////////////////////////////////////////////////////////////////////

        volatile sig_atomic_t _stop;

////////////////////////////////////////////////////////////////////////////////
/// @brief wait interval for the synchronizer thread when idle
////////////////////////////////////////////////////////////////////////////////

        uint64_t const _syncInterval;

////////////////////////////////////////////////////////////////////////////////
/// @brief minimum size for sync operations (in bytes)
////////////////////////////////////////////////////////////////////////////////

        uint64_t const _syncMinSize;

////////////////////////////////////////////////////////////////////////////////
/// @brief time of last sync
////////////////////////////////////////////////////////////////////////////////

        double _lastSyncStamp;

////////////////////////////////////////////////////////////////////////////////
/// @brief total number of syncs performed
////////////////////////////////////////////////////////////////////////////////

        uint64_t _numSyncs;

////////////////////////////////////////////////////////////////////////////////
/// @brief total size of syncs performed
////////////////////////////////////////////////////////////////////////////////

        uint64_t _sizeSyncs;

////////////////////////////////////////////////////////////////////////////////
/// @brief logfile descriptor cache
////////////////////////////////////////////////////////////////////////////////

        struct {
          Logfile::IdType  id;
          int              fd;
        }
        _logfileCache;

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
