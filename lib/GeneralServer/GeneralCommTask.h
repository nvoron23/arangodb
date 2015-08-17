////////////////////////////////////////////////////////////////////////////////
/// @brief task for communications
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
/// @author Achim Brandt
/// @author Copyright 2014, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2009-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGODB_GENERAL_SERVER_GENERAL_COMM_TASK_H
#define ARANGODB_GENERAL_SERVER_GENERAL_COMM_TASK_H 1

#ifdef _WIN32
  #include "Basics/win-utils.h"
#endif
#include "Basics/socket-utils.h"

#include "Basics/Common.h"
#include "Basics/logging.h"

#include "Basics/StringBuffer.h"
#include "Scheduler/SocketTask.h"

// -----------------------------------------------------------------------------
// --SECTION--                                             class GeneralCommTask
// -----------------------------------------------------------------------------

namespace triagens {
  namespace rest {

////////////////////////////////////////////////////////////////////////////////
/// @brief task for general communication
////////////////////////////////////////////////////////////////////////////////

    template<typename S, typename HF>
    class GeneralCommTask : public SocketTask {
      private:
        GeneralCommTask (GeneralCommTask const&);
        GeneralCommTask const& operator= (GeneralCommTask const&);

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief constructs a new task
////////////////////////////////////////////////////////////////////////////////

      public:

        GeneralCommTask (S* server,
                         TRI_socket_t socket,
                         ConnectionInfo const& info,
                         double keepAliveTimeout)
          : Task("GeneralCommTask"),
            SocketTask(socket, keepAliveTimeout),
            _server(server),
            _connectionInfo(info),
            _writeBuffers(),
#ifdef TRI_ENABLE_FIGURES
            _writeBuffersStats(),
#endif
            _readPosition(0),
            _bodyPosition(0),
            _bodyLength(0),
            _requestPending(false),
            _closeRequested(false),
            _readRequestBody(false),
            _request(nullptr),
            _maximalHeaderSize(0),
            _maximalBodySize(0),
            _maximalPipelineSize(0) {

          LOG_TRACE("connection established, client %d, server ip %s, server port %d, client ip %s, client port %d",
                    (int) TRI_get_fd_or_handle_of_socket(socket),
                    _connectionInfo.serverAddress.c_str(),
                    (int) _connectionInfo.serverPort,
                    _connectionInfo.clientAddress.c_str(),
                    (int) _connectionInfo.clientPort);

          const auto p = server->getHandlerFactory()->sizeRestrictions();

          _maximalHeaderSize = p.maximalHeaderSize;
          _maximalBodySize = p.maximalBodySize;
          _maximalPipelineSize = p.maximalPipelineSize;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief destructs a task
////////////////////////////////////////////////////////////////////////////////

      protected:

        ~GeneralCommTask () {
          LOG_TRACE("connection closed, client %d", (int) TRI_get_fd_or_handle_of_socket(_commSocket));

          // free write buffers
          for (std::deque<basics::StringBuffer*>::iterator i = _writeBuffers.begin();  i != _writeBuffers.end();  i++) {
            basics::StringBuffer* buffer = *i;

            delete buffer;
          }

#ifdef TRI_ENABLE_FIGURES

          for (std::deque<TRI_request_statistics_t*>::iterator i = _writeBuffersStats.begin();  i != _writeBuffersStats.end();  i++) {
            TRI_request_statistics_t* buffer = *i;

            TRI_ReleaseRequestStatistics(buffer);
          }

#endif

          // free request
          if (_request != nullptr) {
            delete _request;
          }
        }

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief handles response
////////////////////////////////////////////////////////////////////////////////

        void handleResponse (typename HF::GeneralResponse * response)  {
          _requestPending = false;

          addResponse(response);
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief closes the socket
////////////////////////////////////////////////////////////////////////////////

        void beginShutdown ()  {
          #ifdef _WIN32
            // Can not close socket descriptors here. Probably should not close
            // these for linux as well.
            return;
          #endif
          if (TRI_isvalidsocket(_commSocket)) {
            TRI_CLOSE_SOCKET(_commSocket);
            TRI_invalidatesocket(&_commSocket);
          }
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief reads data from the socket
////////////////////////////////////////////////////////////////////////////////

        virtual bool processRead () = 0;

// -----------------------------------------------------------------------------
// --SECTION--                                         virtual protected methods
// -----------------------------------------------------------------------------

      protected:

////////////////////////////////////////////////////////////////////////////////
/// @brief reads data from the socket
////////////////////////////////////////////////////////////////////////////////

        virtual void addResponse (typename HF::GeneralResponse * response) = 0;

// -----------------------------------------------------------------------------
// --SECTION--                                                 protected methods
// -----------------------------------------------------------------------------

      protected:

////////////////////////////////////////////////////////////////////////////////
/// @brief fills the write buffer
////////////////////////////////////////////////////////////////////////////////

        void fillWriteBuffer () {
          if (! hasWriteBuffer() && ! _writeBuffers.empty()) {
            basics::StringBuffer * buffer = _writeBuffers.front();
            _writeBuffers.pop_front();

#ifdef TRI_ENABLE_FIGURES
            TRI_request_statistics_t* statistics = _writeBuffersStats.front();
            _writeBuffersStats.pop_front();
#else
            TRI_request_statistics_t* statistics = nullptr;
#endif

            setWriteBuffer(buffer, statistics);
          }
        }

// -----------------------------------------------------------------------------
// --SECTION--                                                SocketTask methods
// -----------------------------------------------------------------------------

      protected:

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        bool handleRead (bool& closed)  {
          bool res = true;

          if (! this->_closeRequested) {
            res = fillReadBuffer(closed);

            // process as much data as we got
            while (processRead()) {
              if (this->_closeRequested) {
                break;
              }
            }
          }
          else {
            closed = true;
          }

          if (closed) {
            res = false;
            _server->handleCommunicationClosed(this);
          }
          else if (! res) {
            _server->handleCommunicationFailure(this);
          }

          return res;
        }

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void completedWriteBuffer (bool& closed) {
          _writeBuffer = nullptr;

#ifdef TRI_ENABLE_FIGURES
          if (_writeBufferStatistics != nullptr) {
            _writeBufferStatistics->_writeEnd = TRI_StatisticsTime();

            TRI_ReleaseRequestStatistics(_writeBufferStatistics);
            _writeBufferStatistics = nullptr;
          }
#endif

          fillWriteBuffer();

          if (_closeRequested && ! hasWriteBuffer() && _writeBuffers.empty()) {
            closed = true;
            _server->handleCommunicationClosed(this);
          }
        }

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void handleTimeout () {
          _server->handleCommunicationClosed(this);
        }

// -----------------------------------------------------------------------------
// --SECTION--                                               protected variables
// -----------------------------------------------------------------------------

      protected:

////////////////////////////////////////////////////////////////////////////////
/// @brief the underlying server
////////////////////////////////////////////////////////////////////////////////

        S * const _server;

////////////////////////////////////////////////////////////////////////////////
/// @brief connection info
////////////////////////////////////////////////////////////////////////////////

        ConnectionInfo _connectionInfo;

////////////////////////////////////////////////////////////////////////////////
/// @brief write buffers
////////////////////////////////////////////////////////////////////////////////

        std::deque<basics::StringBuffer*> _writeBuffers;

////////////////////////////////////////////////////////////////////////////////
/// @brief statistics buffers
////////////////////////////////////////////////////////////////////////////////

#ifdef TRI_ENABLE_FIGURES

        std::deque<TRI_request_statistics_t*> _writeBuffersStats;

#endif

////////////////////////////////////////////////////////////////////////////////
/// @brief current read position
////////////////////////////////////////////////////////////////////////////////

        size_t _readPosition;

////////////////////////////////////////////////////////////////////////////////
/// @brief start of the body position
////////////////////////////////////////////////////////////////////////////////

        size_t _bodyPosition;

////////////////////////////////////////////////////////////////////////////////
/// @brief body length
////////////////////////////////////////////////////////////////////////////////

        size_t _bodyLength;

////////////////////////////////////////////////////////////////////////////////
/// @brief true if request is complete but not handled
////////////////////////////////////////////////////////////////////////////////

        bool _requestPending;

////////////////////////////////////////////////////////////////////////////////
/// @brief true if a close has been requested by the client
////////////////////////////////////////////////////////////////////////////////

        bool _closeRequested;

////////////////////////////////////////////////////////////////////////////////
/// @brief true if reading the request body
////////////////////////////////////////////////////////////////////////////////

        bool _readRequestBody;

////////////////////////////////////////////////////////////////////////////////
/// @brief the request with possible incomplete body
////////////////////////////////////////////////////////////////////////////////

        typename HF::GeneralRequest* _request;

////////////////////////////////////////////////////////////////////////////////
/// @brief the maximal header size
////////////////////////////////////////////////////////////////////////////////

        size_t _maximalHeaderSize;

////////////////////////////////////////////////////////////////////////////////
/// @brief the maximal body size
////////////////////////////////////////////////////////////////////////////////

        size_t _maximalBodySize;

////////////////////////////////////////////////////////////////////////////////
/// @brief the maximal pipeline size
////////////////////////////////////////////////////////////////////////////////

        size_t _maximalPipelineSize;
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
