'use strict';

////////////////////////////////////////////////////////////////////////////////
/// @brief Foxx Request Context
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2013 triagens GmbH, Cologne, Germany
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
/// Copyright holder is triAGENS GmbH, Cologne, Germany
///
/// @author Lucas Dohmen
/// @author Michael Hackstein
/// @author Copyright 2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

var SwaggerDocs = require("org/arangodb/foxx/swaggerDocs").Docs,
  joi = require("joi"),
  _ = require("underscore"),
  extend = _.extend,
  internal = require("org/arangodb/foxx/internals"),
  toJSONSchema = require("org/arangodb/foxx/schema").toJSONSchema,
  is = require("org/arangodb/is"),
  BadRequest = require("http-errors").BadRequest,
  UnauthorizedError = require("org/arangodb/foxx/authentication").UnauthorizedError;

function createBodyParamExtractor(rootElement, paramName, allowInvalid) {
  var extractElement;

  if (rootElement) {
    extractElement = function (req) {
      return req.body()[paramName];
    };
  } else {
    extractElement = function (req) {
      return req.body();
    };
  }

  if (!allowInvalid) {
    return extractElement;
  }

  return function (req) {
    try {
      return extractElement(req);
    } catch (e) {
      return {};
    }
  };
}

function createModelInstantiator(Model, allowInvalid) {
  var multiple = is.array(Model);
  Model = multiple ? Model[0] : Model;
  var instantiate = function (raw) {
    if (!allowInvalid) {
      raw = validateOrThrow(raw, Model.prototype.schema, allowInvalid);
    }
    return new Model(raw);
  };
  if (!multiple) {
    return instantiate;
  }
  return function (raw) {
    return _.map(raw, instantiate);
  };
}

function isJoi(schema) {
  if (!schema || typeof schema !== 'object' || is.array(schema)) {
    return false;
  }

  if (schema.isJoi) { // shortcut for pure joi schemas
    return true;
  }

  return Object.keys(schema).some(function (key) {
    return schema[key].isJoi;
  });
}

function validateOrThrow(raw, schema, allowInvalid) {
  if (!isJoi(schema)) {
    return raw;
  }
  var result = joi.validate(raw, schema);
  if (result.error && !allowInvalid) {
    throw new BadRequest(result.error.message.replace(/^"value"/, 'Request body'));
  }
  return result.value;
}

////////////////////////////////////////////////////////////////////////////////
/// @fn JSF_foxx_RequestContext_initializer
/// @brief Context of a Request Definition
///
/// Used for documenting and constraining the routes.
////////////////////////////////////////////////////////////////////////////////

function RequestContext(executionBuffer, models, route, path, rootElement, constraints, extensions) {
  this.path = path;
  this.route = route;
  this.typeToRegex = {
    "int": "/[0-9]+/",
    "integer": "/[0-9]+/",
    "string": "/[^/]+/"
  };
  this.rootElement = rootElement;
  this.constraints = constraints;

  this.docs = new SwaggerDocs(this.route.docs, models);
  this.docs.addNickname(route.docs.httpMethod, route.url.match);

  var attr;
  var extensionWrapper = function(scope, func) {
    return function() {
      func.apply(this, arguments);
      return this;
    }.bind(scope);
  };
  for (attr in extensions) {
    if (extensions.hasOwnProperty(attr)) {
      this[attr] = extensionWrapper(this, extensions[attr]);
    }
  }
  executionBuffer.applyEachFunction(this);
}

extend(RequestContext.prototype, {

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock JSF_foxx_RequestContext_pathParam
///
/// `Route.pathParam(id, options)`
///
/// If you defined a route "/foxx/:name", containing a parameter called `name` you can
/// constrain which format this parameter is allowed to have.
/// This format is defined using *joi* in the `options` parameter.
/// Using this function will at first allow you to access this parameter in your
/// route handler using `req.params(id)`, will reject any request having a paramter
/// that does not match the *joi* definition and creates a documentation for this
/// parameter in ArangoDBs WebInterface.
///
/// For more information on *joi* see [the official Joi documentation](https://github.com/spumko/joi).
///
/// *Parameter*
///
/// * *id*: name of the param.
/// * *options*: a joi schema or an object with the following properties:
///  * *type*: a joi schema.
///  * *description*: documentation description for the parameter.
///  * *required* (optional): whether the parameter is required. Default: determined by *type*.
///
/// *Examples*
///
/// ```js
/// app.get("/foxx/:name", function {
///   // Do something
/// }).pathParam("name", joi.number().integer().required().description("Name of the Foxx"));
/// ```
///
/// You can also pass in a configuration object instead:
///
/// ```js
/// app.get("/foxx/:name", function {
///   // Do something
/// }).pathParam("name", {
///   type: joi.number().integer(),
///   required: true,
///   description: "Name of the Foxx"
/// });
/// ```
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

  pathParam: function (paramName, attributes) {
    var url = this.route.url,
      urlConstraint = url.constraint || {},
      type = attributes.type,
      required = attributes.required,
      description = attributes.description,
      constraint, regexType, cfg;

    if (attributes.isJoi) {
      type = attributes;
      required = undefined;
      description = undefined;
    }

    constraint = type;
    regexType = type;

    if (type) {
      if (typeof required === 'boolean') {
        constraint = required ? constraint.required() : constraint.optional();
      }
      if (typeof description === 'string') {
        constraint = constraint.description(description);
      }
      this.constraints.urlParams[paramName] = constraint;
      cfg = constraint.describe();
      if (is.array(cfg)) {
        cfg = cfg[0];
        type = 'string';
      } else {
        type = cfg.type;
      }
      required = Boolean(cfg.flags && cfg.flags.presence === 'required');
      description = cfg.description;
      if (
        type === 'number' &&
          _.isArray(cfg.rules) &&
          _.some(cfg.rules, function (rule) {
            return rule.name === 'integer';
          })
      ) {
        type = 'integer';
      }
      if (_.has(this.typeToRegex, type)) {
        regexType = type;
      } else {
        regexType = 'string';
      }
    }

    urlConstraint[paramName] = this.typeToRegex[regexType];
    if (!urlConstraint[paramName]) {
      throw new Error("Illegal attribute type: " + regexType);
    }
    this.route.url = internal.constructUrlObject(url.match, urlConstraint, url.methods[0]);
    this.docs.addPathParam(paramName, description, type, required);
    return this;
  },

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock JSF_foxx_RequestContext_queryParam
///
/// `Route.queryParam(id, options)`
///
/// Describe a query parameter:
///
/// If you defined a route "/foxx", you can allow a query paramter with the
/// name `id` on it and constrain the format of this parameter by giving it a *joi* type in the `options` parameter.
/// Using this function will at first allow you to access this parameter in your
/// route handler using `req.params(id)`, will reject any request having a paramter
/// that does not match the *joi* definition and creates a documentation for this
/// parameter in ArangoDBs WebInterface.
///
/// For more information on *joi* see [the official Joi documentation](https://github.com/spumko/joi).
///
/// You can also provide a description of this parameter and
/// whether you can provide the parameter multiple times.
///
/// *Parameter*
///
/// * *id*: name of the parameter
/// * *options*: a joi schema or an object with the following properties:
///  * *type*: a joi schema
///  * *description*: documentation description for this param.
///  * *required* (optional): whether the param is required. Default: determined by *type*.
///  * *allowMultiple* (optional): whether the param can be specified more than once. Default: `false`.
///
/// *Examples*
///
/// ```js
/// app.get("/foxx", function {
///   // Do something
/// }).queryParam("id",
///   joi.number().integer()
///   .required()
///   .description("Id of the Foxx")
///   .meta({allowMultiple: false})
/// });
/// ```
///
/// You can also pass in a configuration object instead:
///
/// ```js
/// app.get("/foxx", function {
///   // Do something
/// }).queryParam("id", {
///   type: joi.number().integer().required().description("Id of the Foxx"),
///   allowMultiple: false
/// });
/// ```
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

  queryParam: function (paramName, attributes) {
    var type = attributes.type,
      required = attributes.required,
      description = attributes.description,
      allowMultiple = attributes.allowMultiple,
      constraint, cfg;

    if (attributes.isJoi) {
      type = attributes;
      required = undefined;
      description = undefined;
      allowMultiple = undefined;
    }

    constraint = type;

    if (type) {
      if (typeof required === 'boolean') {
        constraint = required ? constraint.required() : constraint.optional();
      }
      if (typeof description === 'string') {
        constraint = constraint.description(description);
      }
      if (typeof allowMultiple === 'boolean') {
        constraint = constraint.meta({allowMultiple: allowMultiple});
      }
      this.constraints.queryParams[paramName] = constraint;
      cfg = constraint.describe();
      if (is.array(cfg)) {
        cfg = cfg[0];
        type = 'string';
      } else {
        type = cfg.type;
      }
      required = Boolean(cfg.flags && cfg.flags.presence === 'required');
      description = cfg.description;
      if (cfg.meta) {
        if (!is.array(cfg.meta)) {
          cfg.meta = [cfg.meta];
        }
        _.each(cfg.meta, function (meta) {
          if (meta && typeof meta.allowMultiple === 'boolean') {
            allowMultiple = meta.allowMultiple;
          }
        });
      }
      if (
        type === 'number' &&
          _.isArray(cfg.rules) &&
          _.some(cfg.rules, function (rule) {
            return rule.name === 'integer';
          })
      ) {
        type = 'integer';
      }
    }

    this.docs.addQueryParam(
      paramName,
      description,
      type,
      required,
      Boolean(allowMultiple)
    );
    return this;
  },

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock JSF_foxx_RequestContext_bodyParam
///
/// `Route.bodyParam(paramName, options)`
///
/// Defines that this route expects a JSON body when requested and binds it to
/// a pseudo parameter with the name `paramName`.
/// The body can than be read in the the handler using `req.params(paramName)` on the request object.
/// In the `options` parameter you can define how a valid request body should look like.
/// This definition can be done in two ways, either using *joi* directly.
/// Accessing the body in this case will give you a JSON object.
/// The other way is to use a Foxx *Model*.
/// Accessing the body in this case will give you an instance of this Model.
/// For both ways an entry for the body will be added in the Documentation in ArangoDBs WebInterface.
/// For information about how to annotate your models, see the Model section.
/// All requests sending a body that does not match the validation given this way
/// will automatically be rejected.
///
/// You can also wrap the definition into an array, in this case this route
/// expects a body of type array containing arbitrary many valid objects.
/// Accessing the body parameter will then of course return an array of objects.
///
/// Note: The behavior of `bodyParam` changes depending on the `rootElement` option
/// set in the [manifest](../Develop/Manifest.md). If it is set to `true`, it is
/// expected that the body is an
/// object with a key of the same name as the `paramName` argument.
/// The value of this object is either a single object or in the case of a multi
/// element an array of objects.
///
/// *Parameter*
///
///  * *paramName*: name of the body parameter in `req.parameters`.
///  * *options*: a joi schema or an object with the following properties:
///   * *description*: the documentation description of the request body.
///   * *type*: the Foxx model or joi schema to use.
///   * *allowInvalid* (optional): `true` if validation should be skipped. (Default: `false`)
///
/// *Examples*
///
/// ```js
/// app.post("/foxx", function (req, res) {
///   var foxxBody = req.parameters.foxxBody;
///   // Do something with foxxBody
/// }).bodyParam("foxxBody", {
///   description: "Body of the Foxx",
///   type: FoxxBodyModel
/// });
/// ```
///
/// Using a joi schema:
///
/// ```js
/// app.post("/foxx", function (req, res) {
///   var joiBody = req.parameters.joiBody;
///   // Do something with the number
/// }).bodyParam("joiBody", {
///   type: joi.number().integer().min(5),
///   description: "A number greater than five",
///   allowInvalid: false // default
/// });
/// ```
///
/// Shorthand version:
///
/// ```js
/// app.post("/foxx", function (req, res) {
///   var joiBody = req.parameters.joiBody;
///   // Do something with the number
/// }).bodyParam(
///   "joiBody",
///   joi.number().integer().min(5)
///   .description("A number greater than five")
///   .meta({allowInvalid: false}) // default
/// );
/// ```
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

  bodyParam: function (paramName, attributes) {
    var type = attributes.type,
      description = attributes.description,
      allowInvalid = attributes.allowInvalid,
      cfg, construct;

    if (attributes.isJoi) {
      type = attributes;
      description = undefined;
      allowInvalid = undefined;
    }

    if (!type) {
      construct = function (raw) {
        return raw;
      };
    } else if (typeof type === 'function' || is.array(type)) {
      // assume ModelOrSchema is a Foxx Model
      construct = createModelInstantiator(type, allowInvalid);
    } else {
      if (!type.isJoi) {
        type = joi.object().keys(type).required();
      }
      if (typeof allowInvalid === 'boolean') {
        type = type.meta({allowInvalid: allowInvalid});
      }
      if (typeof description === 'string') {
        type = type.description(description);
      }
      cfg = type.describe();
      description = cfg.description;
      if (cfg.meta) {
        if (!is.array(cfg.meta)) {
          cfg.meta = [cfg.meta];
        }
        _.each(cfg.meta, function (meta) {
          if (meta && typeof meta.allowInvalid === 'boolean') {
            allowInvalid = meta.allowInvalid;
          }
        });
      }
      construct = function (raw) {
        return validateOrThrow(raw, type, allowInvalid);
      };
    }

    this.docs.addBodyParam(
      paramName,
      description,
      toJSONSchema(paramName, is.array(type) ? type[0] : type)
    );
    this.route.action.bodyParams.push({
      extract: createBodyParamExtractor(this.rootElement, paramName, allowInvalid),
      paramName: paramName,
      construct: construct
    });
    return this;
  },

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock JSF_foxx_RequestContext_summary
///
/// `Route.summary(description)`
///
/// Set the summary for this route in the documentation.
/// Can't be longer than 8192 characters.
/// This is equal to using JavaDoc style comments right above your function.
/// If you provide both comment and `summary()` the call to `summary()` wins
/// and will be used.
///
/// *Examples*
///
/// Version with comment:
///
/// ```js
/// /** Short description
///  * 
///  * Longer description
///  * with multiple lines
///  */
/// app.get("/foxx", function() {
/// });
/// ```
/// 
/// is identical to:
///
/// ```js
/// app.get("/foxx", function() {
/// })
/// .summary("Short description")
/// .notes(["Longer description", "with multiple lines"]); 
/// ```
///
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

  summary: function (summary) {
    if (summary.length > 8192) {
      throw new Error("Summary can't be longer than 8192 characters");
    }
    this.docs.addSummary(summary);
    return this;
  },

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock JSF_foxx_RequestContext_notes
///
/// `Route.notes(...description)`
///
/// Set the long description for this route in the documentation
//
/// *Examples*
///
/// Version with comment:
///
/// ```js
/// /** Short description
///  * 
///  * Longer description
///  * with multiple lines
///  */
/// app.get("/foxx", function() {
/// });
/// ```
/// 
/// is identical to:
///
/// ```js
/// app.get("/foxx", function() {
/// })
/// .summary("Short description")
/// .notes(["Longer description", "with multiple lines"]); 
/// ```
///
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////

  notes: function () {
    var notes = Array.prototype.join.call(arguments, '\n');
    this.docs.addNotes(notes);
    return this;
  },

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock JSF_foxx_RequestContext_errorResponse
///
/// `Route.errorResponse(errorClassOrName, code, description, [callback])`
///
/// Define a reaction to a thrown error for this route: If your handler throws an error
/// of the errorClass defined in `errorClassOrName` or the error has an attribute `name` equal to `errorClassOrName`,
/// it will be caught and the response object will be filled with the given
/// status code and a JSON with error set to your description as the body.
///
/// If you want more control over the returned JSON, you can give an optional fourth
/// parameter in form of a function. It gets the error as an argument, the return
/// value will be transformed into JSON and then be used as the body.
/// The status code will be used as described above. The description will be used for
/// the documentation.
///
/// It also adds documentation for this error response to the generated documentation.
///
/// *Examples*
///
/// ```js
/// /* define our own error type, FoxxyError */
/// var FoxxyError = function (message) {
///   this.name = "FError";
///   this.message = "the following FoxxyError occurred: " + message;
/// };
/// FoxxyError.prototype = new Error();
///
/// app.get("/foxx", function {
///   /* throws a FoxxyError */
///   throw new FoxxyError();
/// }).errorResponse(FoxxyError, 303, "This went completely wrong. Sorry!");
///
/// app.get("/foxx", function {
///   throw new FoxxyError("oops!");
/// }).errorResponse("FError", 303, "This went completely wrong. Sorry!", function (e) {
///   return {
///     code: 123,
///     desc: e.message
///   };
/// });
/// ```
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////
  errorResponse: function (errorClass, code, reason, errorHandler) {
    this.route.action.errorResponses.push({
      errorClass: errorClass,
      code: code,
      reason: reason,
      errorHandler: errorHandler
    });
    this.route.docs.errorResponses.push(internal.constructErrorResponseDoc(code, reason));
    return this;
  },

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock JSF_foxx_RequestContext_onlyIf
///
/// `Route.onlyIf(check)`
///
/// This functionality is used to secure a route by applying a checking function
/// on the request beforehand, for example the check authorization.
/// It expects `check` to be a function that takes the request object as first parameter.
/// This function is executed before the actual handler is invoked.
/// If `check` throws an error the actual handler will not be invoked.
/// Remember to provide an `errorResponse` on the route as well to define the behavior in this case.
///
/// *Examples*
///
/// ```js
/// app.get("/foxx", function {
///   // Do something
/// }).onlyIf(aFunction).errorResponse(ErrorClass, 303, "This went completely wrong. Sorry!");
/// ```
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////
  onlyIf: function (check) {
    this.route.action.checks.push({
      check: check
    });
    return this;
  },

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock JSF_foxx_RequestContext_onlyIfAuthenticated
///
/// `FoxxController#onlyIfAuthenticated(code, reason)`
///
/// Please activate sessions for this app if you want to use this function.
/// Or activate authentication (deprecated).
/// If the user is logged in, it will do nothing. Otherwise it will respond with
/// the status code and the reason you provided (the route handler won't be called).
/// This will also add the according documentation for this route.
///
/// *Examples*
///
/// ```js
/// app.get("/foxx", function {
///   // Do something
/// }).onlyIfAuthenticated(401, "You need to be authenticated");
/// ```
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////
  onlyIfAuthenticated: function (code, reason) {
    var check;

    check = function (req) {
      if (
        !(req.session && req.session.get('uid')) // new and shiny
          && !(req.user && req.currentSession) // old and busted
      ) {
        throw new UnauthorizedError();
      }
    };

    if (is.notExisty(code)) {
      code = 401;
    }
    if (is.notExisty(reason)) {
      reason = "Not authorized";
    }

    this.onlyIf(check);
    this.errorResponse(UnauthorizedError, code, reason);

    return this;
  }
});

function RequestContextBuffer() {
  this.applyChain = [];
}

extend(RequestContextBuffer.prototype, {
  applyEachFunction: function (target) {
    _.each(this.applyChain, function (x) {
      target[x.functionName].apply(target, x.argumentList);
    });
  }
});

_.each([
////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock JSF_foxx_RequestContextBuffer_errorResponse
///
/// `Controller.allRoutes.errorResponse(errorClass, code, description)`
///
/// This is equal to invoking `Route.errorResponse` on all routes bound to this controller.
///
/// *Examples*
///
/// ```js
/// app.allRoutes.errorResponse(FoxxyError, 303, "This went completely wrong. Sorry!");
///
/// app.get("/foxx", function {
///   // Do something
/// });
/// ```
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////
  "errorResponse",

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock JSF_foxx_RequestContextBuffer_onlyIf
///
/// `Controller.allRoutes.onlyIf(code, reason)`
///
/// This is equal to invoking `Route.onlyIf` on all routes bound to this controller.
///
/// *Examples*
///
/// ```js
/// app.allRoutes.onlyIf(myPersonalCheck);
///
/// app.get("/foxx", function {
///   // Do something
/// });
/// ```
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////
  "onlyIf",

////////////////////////////////////////////////////////////////////////////////
/// @startDocuBlock JSF_foxx_RequestContextBuffer_onlyIfAuthenticated
///
/// `RequestContextBuffer#errorResponse(errorClass, code, description)`
///
/// This is equal to invoking `Route.onlyIfAuthenticated` on all routes bound to this controller.
///
/// *Examples*
///
/// ```js
/// app.allRoutes.onlyIfAuthenticated(401, "You need to be authenticated");
///
/// app.get("/foxx", function {
///   // Do something
/// });
/// ```
/// @endDocuBlock
////////////////////////////////////////////////////////////////////////////////
  "onlyIfAuthenticated"
], function (functionName) {
  extend(RequestContextBuffer.prototype[functionName] = function () {
    this.applyChain.push({
      functionName: functionName,
      argumentList: arguments
    });
    return this;
  });
});

exports.RequestContext = RequestContext;
exports.RequestContextBuffer = RequestContextBuffer;

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

/// Local Variables:
/// mode: outline-minor
/// outline-regexp: "/// @brief\\|/// @addtogroup\\|/// @page\\|// --SECTION--\\|/// @\\}\\|/\\*jslint"
/// End:
