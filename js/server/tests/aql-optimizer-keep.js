/*jshint globalstrict:false, strict:false, maxlen: 500 */
/*global assertTrue, assertFalse, assertEqual, AQL_EXECUTE */

////////////////////////////////////////////////////////////////////////////////
/// @brief tests for COLLECT w/ KEEP
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2010-2012 triagens GmbH, Cologne, Germany
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
/// @author Jan Steemann
/// @author Copyright 2012, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

var jsunity = require("jsunity");
var internal = require("internal");
var errors = internal.errors;
var db = require("org/arangodb").db;
var helper = require("org/arangodb/aql-helper");
var assertQueryError = helper.assertQueryError;

////////////////////////////////////////////////////////////////////////////////
/// @brief test suite
////////////////////////////////////////////////////////////////////////////////

function optimizerKeepTestSuite () {
  var c;

  return {
    setUp : function () {
      db._drop("UnitTestsCollection");
      c = db._create("UnitTestsCollection");

      for (var i = 0; i < 1000; ++i) {
        c.save({ group: "test" + (i % 10), value: i });
      }
    },

    tearDown : function () {
      db._drop("UnitTestsCollection");
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief test no into, no keep
////////////////////////////////////////////////////////////////////////////////

    testInvalidSyntax : function () {
      assertQueryError(errors.ERROR_QUERY_PARSE.code, "LET a = 1 FOR i IN " + c.name() + " COLLECT class = i.group KEEP i RETURN class");
      assertQueryError(errors.ERROR_QUERY_PARSE.code, "LET a = 1 FOR i IN " + c.name() + " COLLECT class = i.group INTO group KEEP RETURN class");
      assertQueryError(errors.ERROR_QUERY_PARSE.code, "LET a = 1 FOR i IN " + c.name() + " COLLECT class = i.group INTO group KEEP i KEEP i RETURN class");
      assertQueryError(errors.ERROR_QUERY_PARSE.code, "LET a = 1 FOR i IN " + c.name() + " COLLECT class = i.group INTO group KEEP i + 1 RETURN class");
      assertQueryError(errors.ERROR_QUERY_PARSE.code, "LET a = 1 FOR i IN " + c.name() + " COLLECT KEEP i RETURN class");
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief test no into, no keep
////////////////////////////////////////////////////////////////////////////////

    testNoInto : function () {
      var query = "LET a = 1 LET b = 2 LET c = CONCAT('foo', 'bar') FOR i IN " + c.name() + " COLLECT class = i.group RETURN class";

      var results = AQL_EXECUTE(query);
      assertEqual([ "test0", "test1", "test2", "test3", "test4", "test5", "test6", "test7", "test8", "test9" ], results.json, query);
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief test no keep
////////////////////////////////////////////////////////////////////////////////

    testIntoNoKeep : function () {
      var query = "LET a = 1 LET b = 2 LET c = CONCAT('foo', 'bar') FOR i IN " + c.name() + " LET calc = PASSTHRU(i.group) COLLECT class = calc INTO group RETURN group";

      var results = AQL_EXECUTE(query);
      assertEqual(10, results.json.length);
      for (var i = 0; i < results.json.length; ++i) {
        var group = results.json[i];
        assertTrue(Array.isArray(group));
        assertTrue(100, group.length);
        for (var j = 0; j < group.length; ++j) {
          assertFalse(group[j].hasOwnProperty("a"));
          assertFalse(group[j].hasOwnProperty("b"));
          assertFalse(group[j].hasOwnProperty("c"));
          assertTrue(group[j].hasOwnProperty("calc"));
          assertFalse(group[j].hasOwnProperty("class"));
        }
      }
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief test keep
////////////////////////////////////////////////////////////////////////////////

    testIntoKeepRepeated : function () {
      var query = "LET a = 1 LET b = 2 LET c = CONCAT('foo', 'bar') FOR i IN " + c.name() + " COLLECT class = i.group INTO group KEEP a, a, a, a RETURN group";

      var results = AQL_EXECUTE(query);
      assertEqual(10, results.json.length);
      for (var i = 0; i < results.json.length; ++i) {
        var group = results.json[i];
        assertTrue(Array.isArray(group));
        assertTrue(100, group.length);
        for (var j = 0; j < group.length; ++j) {
          assertTrue(group[j].hasOwnProperty("a"));
          assertFalse(group[j].hasOwnProperty("b"));
          assertFalse(group[j].hasOwnProperty("c"));
          assertFalse(group[j].hasOwnProperty("calc"));
          assertFalse(group[j].hasOwnProperty("class"));
        }
      }
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief test keep
////////////////////////////////////////////////////////////////////////////////

    testIntoKeep1 : function () {
      var query = "LET a = 1 LET b = 2 LET c = CONCAT('foo', 'bar') FOR i IN " + c.name() + " LET calc1 = PASSTHRU(i.group) LET calc2 = PASSTHRU(i.group) COLLECT class = calc1 INTO group KEEP calc1 RETURN group";

      var results = AQL_EXECUTE(query);
      assertEqual(10, results.json.length);
      for (var i = 0; i < results.json.length; ++i) {
        var group = results.json[i];
        assertTrue(Array.isArray(group));
        assertTrue(100, group.length);
        for (var j = 0; j < group.length; ++j) {
          assertFalse(group[j].hasOwnProperty("a"));
          assertFalse(group[j].hasOwnProperty("b"));
          assertFalse(group[j].hasOwnProperty("c"));
          assertTrue(group[j].hasOwnProperty("calc1"));
          assertFalse(group[j].hasOwnProperty("calc2"));
          assertFalse(group[j].hasOwnProperty("class"));
        }
      }
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief test keep
////////////////////////////////////////////////////////////////////////////////

    testIntoKeep2 : function () {
      var query = "LET a = 1 LET b = 2 LET c = CONCAT('foo', 'bar') FOR i IN " + c.name() + " LET calc1 = PASSTHRU(i.group) LET calc2 = PASSTHRU(i.group) COLLECT class = calc1 INTO group KEEP calc2, c, a RETURN group";

      var results = AQL_EXECUTE(query);
      assertEqual(10, results.json.length);
      for (var i = 0; i < results.json.length; ++i) {
        var group = results.json[i];
        assertTrue(Array.isArray(group));
        assertTrue(100, group.length);
        for (var j = 0; j < group.length; ++j) {
          assertTrue(group[j].hasOwnProperty("a"));
          assertFalse(group[j].hasOwnProperty("b"));
          assertTrue(group[j].hasOwnProperty("c"));
          assertFalse(group[j].hasOwnProperty("calc1"));
          assertTrue(group[j].hasOwnProperty("calc2"));
          assertFalse(group[j].hasOwnProperty("class"));
        }
      }
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief test keep
////////////////////////////////////////////////////////////////////////////////

    testIntoKeep3 : function () {
      var query = "FOR j IN 1..1 FOR i IN " + c.name() + " LET a = PASSTHRU(1) LET b = CONCAT('foo', 'bar') LET c = CONCAT(i.group, 'x') COLLECT class = i.group INTO group KEEP c, b RETURN group";

      var results = AQL_EXECUTE(query);
      assertEqual(10, results.json.length);
      for (var i = 0; i < results.json.length; ++i) {
        var group = results.json[i];
        assertTrue(Array.isArray(group));
        assertTrue(100, group.length);
        for (var j = 0; j < group.length; ++j) {
          assertFalse(group[j].hasOwnProperty("a"));
          assertTrue(group[j].hasOwnProperty("b"));
          assertTrue(group[j].hasOwnProperty("c"));
          assertFalse(group[j].hasOwnProperty("i"));
          assertFalse(group[j].hasOwnProperty("j"));
          assertFalse(group[j].hasOwnProperty("class"));
        }
      }
    }

  };
}

////////////////////////////////////////////////////////////////////////////////
/// @brief executes the test suite
////////////////////////////////////////////////////////////////////////////////

jsunity.run(optimizerKeepTestSuite);

return jsunity.done();

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// @addtogroup\\|// --SECTION--\\|/// @page\\|/// @}\\)"
// End:
