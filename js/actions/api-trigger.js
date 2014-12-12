/*jshint strict: false */
/*global require */

////////////////////////////////////////////////////////////////////////////////
/// @brief trigger management
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2014 ArangoDB GmbH, Cologne, Germany
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
////////////////////////////////////////////////////////////////////////////////

var arangodb = require("org/arangodb");
var actions = require("org/arangodb/actions");

var API = "_api/trigger";

// -----------------------------------------------------------------------------
// --SECTION--                                                  public functions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock JSF_get_api_triggers
/// @brief returns all trigger of a collection
///
/// @RESTHEADER{GET /_api/trigger, Read all triggers of a collection}
///
/// @RESTQUERYPARAMETERS
///
/// @RESTQUERYPARAM{collection,string,required}
/// The collection name.
///
/// @RESTDESCRIPTION
///
/// Returns an object with an attribute *triggers* containing a list of all
/// trigger descriptions for the given collection.
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

function get_api_triggers (req, res) {
  var name = req.parameters.collection;
  var collection = arangodb.db._collection(name);

  if (collection === null) {
    actions.collectionNotFound(req, res, name);
    return;
  }

  var triggers = collection.getTriggers();
  var result = { triggers: triggers };

  actions.resultOk(req, res, actions.HTTP_OK, result);
}

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock JSF_get_api_trigger
/// @brief returns a trigger
///
/// @RESTHEADER{GET /_api/trigger/{trigger-handle},Read trigger}
///
/// @RESTURLPARAMETERS
///
/// @RESTURLPARAM{trigger-handle,string,required}
/// The trigger-handle.
///
/// @RESTDESCRIPTION
///
/// The result is an object describing the trigger. It has at least the following
/// attributes:
///
/// - *id*: The identifier of the trigger.
///
/// - *type*: The trigger type (*insert*, *update*, *replace* or *remove*)
///
/// - *position*: The trigger execution position (*pre* or *post*)
///
/// @RESTRETURNCODES
///
/// @RESTRETURNCODE{200}
/// If the trigger exists, then a *HTTP 200* is returned.
///
/// @RESTRETURNCODE{404}
/// If the trigger does not exist, then a *HTTP 404* is returned.
////////////////////////////////////////////////////////////////////////////////

function get_api_trigger (req, res) {

  // .............................................................................
  // /_api/trigger?collection=<collection-name>
  // .............................................................................

  if (req.suffix.length === 0) {
    get_api_triggers(req, res);
  }

  // .............................................................................
  // /_api/trigger/<collection-name>/<trigger-identifier>
  // .............................................................................

  else if (req.suffix.length === 2) {
    var name = decodeURIComponent(req.suffix[0]);
    var collection = arangodb.db._collection(name);

    if (collection === null) {
      actions.collectionNotFound(req, res, name);
      return;
    }

    var tid = decodeURIComponent(req.suffix[1]);
    try {
      var trigger = collection.trigger(name + "/" + tid);
      if (trigger !== null) {
        actions.resultOk(req, res, actions.HTTP_OK, trigger);
        return;
      }
    }
    catch (err) {
      if (err.errorNum === arangodb.ERROR_ARANGO_TRIGGER_NOT_FOUND ||
          err.errorNum === arangodb.ERROR_ARANGO_COLLECTION_NOT_FOUND) {
        actions.triggerNotFound(req, res, collection, tid);
        return;
      }
      throw err;
    }
  }
  else {
    actions.resultBad(req, res, arangodb.ERROR_HTTP_BAD_PARAMETER,
                      "expect GET /" + API + "/<trigger-handle>");
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock JSF_post_api_trigger
/// @brief create a trigger
///
/// @RESTHEADER{POST /_api/trigger, Create trigger}
///
/// @RESTQUERYPARAMETERS
///
/// @RESTQUERYPARAM{collection,string,required}
/// The collection name.
///
/// @RESTBODYPARAM{trigger-data,json,required}
///
/// @RESTDESCRIPTION
///
/// Creates a new trigger in the collection *collection*. Expects
/// an object containing the trigger details.
///
/// The type of the trigger to be created must specified in the *type*
/// attribute of the trigger data. Additionally, the execution position
/// must be specified in the *position* attribute.
///
/// @RESTRETURNCODES
///
/// @RESTRETURNCODE{201}
/// If the trigger does not already exist and could be created, then an *HTTP 201*
/// is returned.
///
/// @RESTRETURNCODE{400}
/// If an invalid trigger description is posted then an *HTTP 400* is returned.
///
/// @RESTRETURNCODE{404}
/// If *collection* is unknown, then an *HTTP 404* is returned.
/// @endDocuBlock
///
/// @RESTRETURNCODE{409}
/// If a trigger already exists for the specified type and execution position,
/// then *HTTP 409* is returned.
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

function post_api_trigger (req, res) {
  if (req.suffix.length !== 0) {
    actions.resultBad(req, res, arangodb.ERROR_HTTP_BAD_PARAMETER,
                      "expecting POST /" + API + "?collection=<collection-name>");
    return;
  }

  var name = req.parameters.collection;
  var collection = arangodb.db._collection(name);

  if (collection === null) {
    actions.collectionNotFound(req, res, name);
    return;
  }

  var body = actions.getJsonBody(req, res);

  if (body === undefined) {
    return;
  }

  // inject collection name into body
  if (body.collection === undefined) {
    body.collection = name;
  }

  // create the trigger
  var trigger = collection.createTrigger(body);
  actions.resultOk(req, res, actions.HTTP_CREATED, trigger);
}

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock JSF_delete_api_trigger
/// @brief deletes a trigger
///
/// @RESTHEADER{DELETE /_api/trigger/{trigger-handle}, Delete trigger}
///
/// @RESTQUERYPARAMETERS
///
/// @RESTQUERYPARAM{trigger-handle,string,required}
/// The trigger handle.
///
/// @RESTDESCRIPTION
///
/// Deletes a trigger with *trigger-handle*.
///
/// @RESTRETURNCODES
///
/// @RESTRETURNCODE{200}
/// If the trigger could be deleted, then an *HTTP 200* is
/// returned.
///
/// @RESTRETURNCODE{404}
/// If the *trigger-handle* is unknown, then an *HTTP 404* is returned.
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

function delete_api_trigger (req, res) {
  if (req.suffix.length !== 2) {
    actions.resultBad(req, res, arangodb.ERROR_HTTP_BAD_PARAMETER,
                      "expect DELETE /" + API + "/<trigger-handle>");
    return;
  }

  var name = decodeURIComponent(req.suffix[0]);
  var collection = arangodb.db._collection(name);

  if (collection === null) {
    actions.collectionNotFound(req, res, name);
    return;
  }

  var tid = parseInt(decodeURIComponent(req.suffix[1]), 10);
  var dropped = collection.dropTrigger(name + "/" + tid);

  if (dropped) {
    actions.resultOk(req, res, actions.HTTP_OK, { id : name + "/" + tid });
  }
  else {
    actions.triggerNotFound(req, res, collection, name + "/" + tid);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// @brief reads or creates a trigger
////////////////////////////////////////////////////////////////////////////////

actions.defineHttp({
  url : API,

  callback : function (req, res) {
    try {
      if (req.requestType === actions.GET) {
        get_api_trigger(req, res);
      }
      else if (req.requestType === actions.DELETE) {
        delete_api_trigger(req, res);
      }
      else if (req.requestType === actions.POST) {
        post_api_trigger(req, res);
      }
      else {
        actions.resultUnsupported(req, res);
      }
    }
    catch (err) {
      actions.resultException(req, res, err, undefined, false);
    }
  }
});

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
