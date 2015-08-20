/*jshint strict: false */

////////////////////////////////////////////////////////////////////////////////
/// @brief administration actions
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2014-2015 ArangoDB GmbH, Cologne, Germany
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
/// @author Copyright 2012-2014, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

var internal = require("internal");
var console = require("console");

var actions = require("org/arangodb/actions");
var arangodb = require("org/arangodb");

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

// var queue = Foxx.queues.create("internal-demo-queue");

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock JSF_get_admin_time
/// @brief Get the current time of the system
///
/// @RESTHEADER{GET /_admin/time, Return system time}
///
/// @RESTDESCRIPTION
///
/// The call returns an object with the attribute *time*. This contains the
/// current system time as a Unix timestamp with microsecond precision.
///
/// @RESTRETURNCODES
///
/// @RESTRETURNCODE{200}
/// Time was returned successfully.
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

actions.defineHttp({
  url : "_admin/time",
  prefix : false,

  callback : function (req, res) {
    actions.resultOk(req, res, actions.HTTP_OK, { time : internal.time() });
  }
});

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock JSF_get_admin_sleep
/// @brief Suspend the execution for a specified duration before returnig
///
/// @RESTHEADER{GET /_admin/sleep?duration=5, Sleep for 5 seconds}
///
/// @RESTDESCRIPTION
///
/// The call returns an object with the attribute *duration*. This takes
/// as many seconds as the duration argument says.
///
/// @RESTRETURNCODES
///
/// @RESTRETURNCODE{200}
/// Sleep was conducted successfully.
/// @DendocuBlock
////////////////////////////////////////////////////////////////////////////////

actions.defineHttp({
  url : "_admin/sleep",
  prefix : false,

  callback : function (req, res) {
    var time = parseFloat(req.parameters.duration);
    if (isNaN(time)) {
      time = 3.0;
    }
    internal.wait(time);
    actions.resultOk(req, res, actions.HTTP_OK, { duration : time });
  }
});

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock JSF_get_admin_echo
/// @brief Send back what was sent in, headers, post body etc.
///
/// @RESTHEADER{GET /_admin/echo, Return current request}
///
/// @RESTDESCRIPTION
///
/// The call returns an object with the following attributes:
///
/// - *headers*: object with HTTP headers received
///
/// - *requestType*: the HTTP request method (e.g. GET)
///
/// - *parameters*: object with URL parameters received
///
/// @RESTRETURNCODES
///
/// @RESTRETURNCODE{200}
/// Echo was returned successfully.
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

actions.defineHttp({
  url : "_admin/echo",
  prefix : true,

  callback : function (req, res) {
    res.responseCode = actions.HTTP_OK;
    res.contentType = "application/json; charset=utf-8";
    req.rawRequestBody = require('internal').rawRequestBody(req);
    res.body = JSON.stringify(req);
  }
});

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock JSF_get_admin_long_echo
/// @brief Send back what was sent in, headers, post body etc.
///
/// @RESTHEADER{GET /_admin/long_echo, Return current request and continues}
///
/// @RESTDESCRIPTION
///
/// The call returns an object with the following attributes:
///
/// - *headers*: object with HTTP headers received
///
/// - *requestType*: the HTTP request method (e.g. GET)
///
/// - *parameters*: object with URL parameters received
///
/// @RESTRETURNCODES
///
/// @RESTRETURNCODE{200}
/// Echo was returned successfully.
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

actions.defineHttp({
  url : "_admin/long_echo",
  prefix : true,

  callback : function (req, res) {
    require("console").log("long polling request from client %s", req.client.id);

    res.responseCode = actions.HTTP_OK;
    res.contentType = "application/json; charset=utf-8";
    res.headers = { 'transfer-encoding': 'chunked' };

    res.body = JSON.stringify(req);
  }
});

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock JSF_get_admin_statistics
/// @brief return the statistics information
///
/// @RESTHEADER{GET /_admin/statistics, Read the statistics}
///
/// @RESTDESCRIPTION
///
/// Returns the statistics information. The returned object contains the
/// statistics figures grouped together according to the description returned by
/// *_admin/statistics-description*. For instance, to access a figure *userTime*
/// from the group *system*, you first select the sub-object describing the
/// group stored in *system* and in that sub-object the value for *userTime* is
/// stored in the attribute of the same name.
///
/// In case of a distribution, the returned object contains the total count in
/// *count* and the distribution list in *counts*. The sum (or total) of the
/// individual values is returned in *sum*.
///
/// @RESTRETURNCODES
///
/// @RESTRETURNCODE{200}
/// Statistics were returned successfully.
///
/// @EXAMPLES
///
/// @EXAMPLE_ARANGOSH_RUN{RestAdminStatistics1}
///     var url = "/_admin/statistics";
///     var response = logCurlRequest('GET', url);
///
///     assert(response.code === 200);
///
///     logJsonResponse(response);
/// @END_EXAMPLE_ARANGOSH_RUN
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

actions.defineHttp({
  url : "_admin/statistics",
  prefix : false,

  callback : function (req, res) {
    var result;

    try {
      result = {};
      result.time = internal.time();
      result.system = internal.processStatistics();
      result.client = internal.clientStatistics();
      result.http = internal.httpStatistics();
      result.server = internal.serverStatistics();

      actions.resultOk(req, res, actions.HTTP_OK, result);
    }
    catch (err) {
      actions.resultException(req, res, err, undefined, false);
    }
  }
});

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock JSF_get_admin_statistics_description
/// @brief fetch descriptive info of statistics
///
/// @RESTHEADER{GET /_admin/statistics-description, Statistics description}
///
/// @RESTDESCRIPTION
///
/// Returns a description of the statistics returned by */_admin/statistics*.
/// The returned objects contains an array of statistics groups in the attribute
/// *groups* and an array of statistics figures in the attribute *figures*.
///
/// A statistics group is described by
///
/// - *group*: The identifier of the group.
/// - *name*: The name of the group.
/// - *description*: A description of the group.
///
/// A statistics figure is described by
///
/// - *group*: The identifier of the group to which this figure belongs.
/// - *identifier*: The identifier of the figure. It is unique within the group.
/// - *name*: The name of the figure.
/// - *description*: A description of the figure.
/// - *type*: Either *current*, *accumulated*, or *distribution*.
/// - *cuts*: The distribution vector.
/// - *units*: Units in which the figure is measured.
///
/// @RESTRETURNCODES
///
/// @RESTRETURNCODE{200}
/// Description was returned successfully.
///
/// @EXAMPLES
///
/// @EXAMPLE_ARANGOSH_RUN{RestAdminStatisticsDescription1}
///     var url = "/_admin/statistics-description";
///     var response = logCurlRequest('GET', url);
///
///     assert(response.code === 200);
///
///     logJsonResponse(response);
/// @END_EXAMPLE_ARANGOSH_RUN
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

actions.defineHttp({
  url : "_admin/statistics-description",
  prefix : false,

  callback : function (req, res) {
    var result;

    try {
      result = {
        groups: [
          {
            group: "system",
            name: "Process Statistics",
            description: "Statistics about the ArangoDB process"
          },

          {
            group: "client",
            name: "Client Connection Statistics",
            description: "Statistics about the connections."
          },

          {
            group: "http",
            name: "HTTP Request Statistics",
            description: "Statistics about the HTTP requests."
          },

          {
            group: "server",
            name: "Server Statistics",
            description: "Statistics about the ArangoDB server"
          }

        ],

        figures: [

          // .............................................................................
          // system statistics
          // .............................................................................

          {
            group: "system",
            identifier: "userTime",
            name: "User Time",
            description: "Amount of time that this process has been scheduled in user mode, " +
                         "measured in seconds.",
            type: "accumulated",
            units: "seconds"
          },

          {
            group: "system",
            identifier: "systemTime",
            name: "System Time",
            description: "Amount of time that this process has been scheduled in kernel mode, " +
                         "measured in seconds.",
            type: "accumulated",
            units: "seconds"
          },

          {
            group: "system",
            identifier: "numberOfThreads",
            name: "Number of Threads",
            description: "Number of threads in the arangod process.",
            type: "current",
            units: "number"
          },

          {
            group: "system",
            identifier: "residentSize",
            name: "Resident Set Size",
            description: "The total size of the number of pages the process has in real memory. " +
                         "This is just the pages which count toward text, data, or stack space. " +
                         "This does not include pages which have not been demand-loaded in, " +
                         "or which are swapped out. The resident set size is reported in bytes.",
            type: "current",
            units: "bytes"
          },

          {
            group: "system",
            identifier: "residentSizePercent",
            name: "Resident Set Size",
            description: "The percentage of physical memory used by the process as resident " +
                         "set size.",
            type: "current",
            units: "percent"
          },

          {
            group: "system",
            identifier: "virtualSize",
            name: "Virtual Memory Size",
            description: "On Windows, this figure contains the total amount of memory that the " +
                         "memory manager has committed for the arangod process. On other " +
                         "systems, this figure contains The size of the virtual memory the " +
                         "process is using.",
            type: "current",
            units: "bytes"
          },

          {
            group: "system",
            identifier: "minorPageFaults",
            name: "Minor Page Faults",
            description: "The number of minor faults the process has made which have " +
                         "not required loading a memory page from disk. This figure is " +
                         "not reported on Windows.",
            type: "accumulated",
            units: "number"
          },

          {
            group: "system",
            identifier: "majorPageFaults",
            name: "Major Page Faults",
            description: "On Windows, this figure contains the total number of page faults. " +
                         "On other system, this figure contains the number of major faults the " +
                         "process has made which have required loading a memory page from disk.",
            type: "accumulated",
            units: "number"
          },

          // .............................................................................
          // client statistics
          // .............................................................................

          {
            group: "client",
            identifier: "httpConnections",
            name: "Client Connections",
            description: "The number of connections that are currently open.",
            type: "current",
            units: "number"
          },

          {
            group: "client",
            identifier: "totalTime",
            name: "Total Time",
            description: "Total time needed to answer a request.",
            type: "distribution",
            cuts: internal.requestTimeDistribution,
            units: "seconds"
          },

          {
            group: "client",
            identifier: "requestTime",
            name: "Request Time",
            description: "Request time needed to answer a request.",
            type: "distribution",
            cuts: internal.requestTimeDistribution,
            units: "seconds"
          },

          {
            group: "client",
            identifier: "queueTime",
            name: "Queue Time",
            description: "Queue time needed to answer a request.",
            type: "distribution",
            cuts: internal.requestTimeDistribution,
            units: "seconds"
          },

          {
            group: "client",
            identifier: "bytesSent",
            name: "Bytes Sent",
            description: "Bytes sents for a request.",
            type: "distribution",
            cuts: internal.bytesSentDistribution,
            units: "bytes"
          },

          {
            group: "client",
            identifier: "bytesReceived",
            name: "Bytes Received",
            description: "Bytes receiveds for a request.",
            type: "distribution",
            cuts: internal.bytesReceivedDistribution,
            units: "bytes"
          },

          {
            group: "client",
            identifier: "connectionTime",
            name: "Connection Time",
            description: "Total connection time of a client.",
            type: "distribution",
            cuts: internal.connectionTimeDistribution,
            units: "seconds"
          },

          {
            group: "http",
            identifier: "requestsTotal",
            name: "Total requests",
            description: "Total number of HTTP requests.",
            type: "accumulated",
            units: "number"
          },

          {
            group: "http",
            identifier: "requestsAsync",
            name: "Async requests",
            description: "Number of asynchronously executed HTTP requests.",
            type: "accumulated",
            units: "number"
          },

          {
            group: "http",
            identifier: "requestsGet",
            name: "HTTP GET requests",
            description: "Number of HTTP GET requests.",
            type: "accumulated",
            units: "number"
          },

          {
            group: "http",
            identifier: "requestsHead",
            name: "HTTP HEAD requests",
            description: "Number of HTTP HEAD requests.",
            type: "accumulated",
            units: "number"
          },

          {
            group: "http",
            identifier: "requestsPost",
            name: "HTTP POST requests",
            description: "Number of HTTP POST requests.",
            type: "accumulated",
            units: "number"
          },

          {
            group: "http",
            identifier: "requestsPut",
            name: "HTTP PUT requests",
            description: "Number of HTTP PUT requests.",
            type: "accumulated",
            units: "number"
          },

          {
            group: "http",
            identifier: "requestsPatch",
            name: "HTTP PATCH requests",
            description: "Number of HTTP PATCH requests.",
            type: "accumulated",
            units: "number"
          },

          {
            group: "http",
            identifier: "requestsDelete",
            name: "HTTP DELETE requests",
            description: "Number of HTTP DELETE requests.",
            type: "accumulated",
            units: "number"
          },

          {
            group: "http",
            identifier: "requestsOptions",
            name: "HTTP OPTIONS requests",
            description: "Number of HTTP OPTIONS requests.",
            type: "accumulated",
            units: "number"
          },

          {
            group: "http",
            identifier: "requestsOther",
            name: "other HTTP requests",
            description: "Number of other HTTP requests.",
            type: "accumulated",
            units: "number"
          },


          // .............................................................................
          // server statistics
          // .............................................................................

          {
            group: "server",
            identifier: "uptime",
            name: "Server Uptime",
            description: "Number of seconds elapsed since server start.",
            type: "current",
            units: "seconds"
          },

          {
            group: "server",
            identifier: "physicalMemory",
            name: "Physical Memory",
            description: "Physical memory in bytes.",
            type: "current",
            units: "bytes"
          }

        ]
      };

      actions.resultOk(req, res, actions.HTTP_OK, result);
    }
    catch (err) {
      actions.resultException(req, res, err, undefined, false);
    }
  }
});

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock JSF_post_admin_test
/// @brief show the available unittests on the server.
///
/// @RESTHEADER{POST /_admin/test, Runs tests on server}
///
/// @RESTBODYPARAM{body,javascript,required}
/// A JSON object containing an attribute *tests* which lists the files
/// containing the test suites.
///
/// @RESTDESCRIPTION
///
/// Executes the specified tests on the server and returns an object with the
/// test results. The object has an attribute "error" which states whether
/// any error occurred. The object also has an attribute "passed" which
/// indicates which tests passed and which did not.
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

actions.defineHttp({
  url : "_admin/test",
  prefix : false,

  callback : function (req, res) {
    var body = actions.getJsonBody(req, res);

    if (body === undefined) {
      return;
    }

    var tests = body.tests;
    if (! Array.isArray(tests)) {
      actions.resultError(req, res,
                          actions.HTTP_BAD, arangodb.ERROR_HTTP_BAD_PARAMETER,
                          "expected attribute 'tests' is missing");
      return;
    }

    var jsUnity = require("jsunity");
    var testResults = { passed: { }, error: false };

    tests.forEach (function (test) {
      var result = false;
      try {
        result = jsUnity.runTest(test);
      }
      catch (err) {
      }
      testResults.passed[test] = result;
      if (! result) {
        testResults.error = true;
      }
    });

    actions.resultOk(req, res, actions.HTTP_OK, testResults);
  }
});

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock JSF_post_admin_execute
/// @brief Execute a script on the server.
///
/// @RESTHEADER{POST /_admin/execute, Execute program}
///
/// @RESTBODYPARAM{body,javascript,required}
/// The body to be executed.
///
/// @RESTDESCRIPTION
///
/// Executes the javascript code in the body on the server as the body
/// of a function with no arguments. If you have a *return* statement
/// then the return value you produce will be returned as content type
/// *application/json*. If the parameter *returnAsJSON* is set to
/// *true*, the result will be a JSON object describing the return value
/// directly, otherwise a string produced by JSON.stringify will be
/// returned.
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

actions.defineHttp({
  url : "_admin/execute",
  prefix : false,

  callback : function (req, res) {
    /*jshint evil: true */
    var body = req.requestBody;
    var result;

    console.warn("about to execute: '%s'", body);

    if (body !== "") {
      result = eval("(function() {" + body + "}());");
    }

    if (req.parameters.hasOwnProperty("returnAsJSON") &&
        req.parameters.returnAsJSON === "true") {
      actions.resultOk(req, res, actions.HTTP_OK, result);
    }
    else {
      actions.resultOk(req, res, actions.HTTP_OK, JSON.stringify(result));
    }
  }
});

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|/// @startDocuBlock\\|// --SECTION--\\|/// @\\}"
// End:
