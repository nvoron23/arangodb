/*jshint strict: true */
/*global require, fail, assertEqual */

////////////////////////////////////////////////////////////////////////////////
/// @brief test the collection interface
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
var arangodb = require("org/arangodb");
var db = arangodb.db;
var internal = require("internal");

// -----------------------------------------------------------------------------
// --SECTION--                                                   mvcc exceptions
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief test suite
////////////////////////////////////////////////////////////////////////////////

function mvccExceptionsSuite () {
  "use strict";
  
  var cn = "UnitTestsMvcc";
  var c;

  return {

    setUp: function () {
      internal.debugClearFailAt();
      db._drop(cn);
      c = db._create(cn);
    },

    tearDown: function () {
      internal.debugClearFailAt();
      var s = db._stackTransactions().reverse();
      for (var i = 0; i < s.length; ++i) {
        try {
          db._rollbackTransaction(s[i]);
        }
        catch (err) {
        }
      }
      db._drop(cn);
      c = null;
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief test out-of-memory in insertRunningTransaction
////////////////////////////////////////////////////////////////////////////////

    testInsertRunningTransaction1Oom : function () {
      internal.debugSetFailAt("LocalTransactionManager::insertRunningTransaction1");

      try {
        db._beginTransaction();
        fail();
      }
      catch (err) {
        assertEqual(internal.errors.ERROR_OUT_OF_MEMORY.code, err.errorNum);
      }

      internal.debugClearFailAt();
      
      // now it should work
      var trx = db._beginTransaction();
      db._rollbackTransaction(trx.id);
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief test out-of-memory in insertRunningTransaction
////////////////////////////////////////////////////////////////////////////////

    testInsertRunningTransaction2Oom : function () {
      internal.debugSetFailAt("LocalTransactionManager::insertRunningTransaction2");

      try {
        db._beginTransaction();
        fail();
      }
      catch (err) {
        assertEqual(internal.errors.ERROR_OUT_OF_MEMORY.code, err.errorNum);
      }

      internal.debugClearFailAt();
      
      // now it should work
      var trx = db._beginTransaction();
      db._rollbackTransaction(trx.id);
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief test out-of-memory in deleteKilledTransactions
////////////////////////////////////////////////////////////////////////////////

    testDeleteKilledTransactionsOom : function () {
      internal.debugSetFailAt("LocalTransactionManager::deleteKilledTransactions");

      var trx1 = db._beginTransaction({ ttl: 2 });
      var stack;

      stack = db._stackTransactions().map(function(t) { return t.id; });
      assertEqual(1, stack.length);
      assertEqual(trx1.id, stack[0]);
      
      internal.wait(4.0, false);
      
      // transaction should still be on the stack, although expired & killed
      stack = db._stackTransactions().map(function(t) { return t.id; });
      assertEqual(1, stack.length);
      assertEqual(trx1.id, stack[0]);

      internal.debugClearFailAt();
      db._rollbackTransaction(trx1.id);
      
      internal.wait(2.0, false);
      
      // transaction should be gone now
      stack = db._stackTransactions().map(function(t) { return t.id; });
      assertEqual(0, stack.length);
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief test out-of-memory in InsertDocumentWorker
////////////////////////////////////////////////////////////////////////////////

    testInsertDocumentWorker1Oom : function () {
      internal.debugSetFailAt("CollectionOperations::InsertDocumentWorker1");

      try {
        c.mvccInsert({ _key: "abc", value: 1 });
        fail();
      }
      catch (err) {
        assertEqual(internal.errors.ERROR_DEBUG.code, err.errorNum);
      }

      try {
        c.mvccDocument("abc");
        fail();
      }
      catch (err) {
        assertEqual(internal.errors.ERROR_ARANGO_DOCUMENT_NOT_FOUND.code, err.errorNum);
      }
      
      assertEqual(0, c.mvccCount());

      internal.debugClearFailAt();
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief test out-of-memory in InsertDocumentWorker
////////////////////////////////////////////////////////////////////////////////

    testInsertDocumentWorker2Oom : function () {
      internal.debugSetFailAt("CollectionOperations::InsertDocumentWorker2");

      try {
        c.mvccInsert({ _key: "abc", value: 1 });
        fail();
      }
      catch (err) {
        assertEqual(internal.errors.ERROR_DEBUG.code, err.errorNum);
      }

      try {
        c.mvccDocument("abc");
        fail();
      }
      catch (err) {
        assertEqual(internal.errors.ERROR_ARANGO_DOCUMENT_NOT_FOUND.code, err.errorNum);
      }
      
      assertEqual(0, c.mvccCount());

      internal.debugClearFailAt();
    },

////////////////////////////////////////////////////////////////////////////////
/// @brief test out-of-memory in InsertDocumentWorker
////////////////////////////////////////////////////////////////////////////////

    testInsertDocumentWorker3Oom : function () {
      internal.debugSetFailAt("CollectionOperations::InsertDocumentWorker3");

      try {
        c.mvccInsert({ _key: "abc", value: 1 });
        fail();
      }
      catch (err) {
        assertEqual(internal.errors.ERROR_DEBUG.code, err.errorNum);
      }

      try {
        c.mvccDocument("abc");
        fail();
      }
      catch (err) {
        assertEqual(internal.errors.ERROR_ARANGO_DOCUMENT_NOT_FOUND.code, err.errorNum);
      }

      assertEqual(0, c.mvccCount());

      internal.debugClearFailAt();
    }

  };
}

// -----------------------------------------------------------------------------
// --SECTION--                                                              main
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief executes the test suite
////////////////////////////////////////////////////////////////////////////////

if (internal.debugCanUseFailAt()) {
  jsunity.run(mvccExceptionsSuite);
}

return jsunity.done();

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// @addtogroup\\|// --SECTION--\\|/// @page\\|/// @}\\)"
// End:

