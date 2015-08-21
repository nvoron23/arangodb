////////////////////////////////////////////////////////////////////////////////
/// @brief abstract class for http handlers
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2014-2015 ArangoDB GmbH, Cologne, Germany
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
/// @author Copyright 2014-2015, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2009-2014, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGODB_HTTP_SERVER_HTTP_HANDLER_H
#define ARANGODB_HTTP_SERVER_HTTP_HANDLER_H 1

#include "Rest/Handler.h"

#include "Rest/HttpResponse.h"

// -----------------------------------------------------------------------------
// --SECTION--                                              forward declarations
// -----------------------------------------------------------------------------

namespace triagens {
  namespace rest {
    class HttpHandlerFactory;
    class HttpRequest;

// -----------------------------------------------------------------------------
// --SECTION--                                                 class HttpHandler
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief abstract class for http handlers
////////////////////////////////////////////////////////////////////////////////

    class HttpHandler : public Handler {
      HttpHandler (HttpHandler const&) = delete;
      HttpHandler& operator= (HttpHandler const&) = delete;

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief constructs a new handler
///
/// Note that the handler owns the request and the response. It is its
/// responsibility to destroy them both. See also the two steal methods.
////////////////////////////////////////////////////////////////////////////////

        explicit
        HttpHandler (HttpRequest*);

////////////////////////////////////////////////////////////////////////////////
/// @brief destructs a handler
////////////////////////////////////////////////////////////////////////////////

        ~HttpHandler ();

// -----------------------------------------------------------------------------
// --SECTION--                                            virtual public methods
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief adds a response
////////////////////////////////////////////////////////////////////////////////

        virtual void addResponse (HttpHandler*);

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief register the server object
////////////////////////////////////////////////////////////////////////////////

        void setServer (HttpHandlerFactory* server);

////////////////////////////////////////////////////////////////////////////////
/// @brief return a pointer to the request
////////////////////////////////////////////////////////////////////////////////

        HttpRequest const* getRequest () const;

////////////////////////////////////////////////////////////////////////////////
/// @brief steal the pointer to the request
////////////////////////////////////////////////////////////////////////////////

        HttpRequest* stealRequest ();

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the response
////////////////////////////////////////////////////////////////////////////////

        HttpResponse* getResponse () const;

////////////////////////////////////////////////////////////////////////////////
/// @brief steal the response
////////////////////////////////////////////////////////////////////////////////

        HttpResponse* stealResponse ();

// -----------------------------------------------------------------------------
// --SECTION--                                                   Handler methods
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        std::string const& queue () const override;

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        Job* createJob (HttpServer*, bool isDetached) override;

// -----------------------------------------------------------------------------
// --SECTION--                                                 protected methods
// -----------------------------------------------------------------------------

      protected:

////////////////////////////////////////////////////////////////////////////////
/// @brief ensure the handler has only one response, otherwise we'd have a leak
////////////////////////////////////////////////////////////////////////////////

        void removePreviousResponse ();

////////////////////////////////////////////////////////////////////////////////
/// @brief create a new HTTP response
////////////////////////////////////////////////////////////////////////////////

        HttpResponse* createResponse (HttpResponse::HttpResponseCode);

// -----------------------------------------------------------------------------
// --SECTION--                                               protected variables
// -----------------------------------------------------------------------------

      protected:

////////////////////////////////////////////////////////////////////////////////
/// @brief the request
////////////////////////////////////////////////////////////////////////////////

        HttpRequest* _request;

////////////////////////////////////////////////////////////////////////////////
/// @brief the response
////////////////////////////////////////////////////////////////////////////////

        HttpResponse* _response;

////////////////////////////////////////////////////////////////////////////////
/// @brief the server
////////////////////////////////////////////////////////////////////////////////

        HttpHandlerFactory* _server;

////////////////////////////////////////////////////////////////////////////////
/// @brief the queue
////////////////////////////////////////////////////////////////////////////////

        std::string _queueName;
    };
  }
}

#endif

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------
