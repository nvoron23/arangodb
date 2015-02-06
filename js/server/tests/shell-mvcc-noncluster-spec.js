////////////////////////////////////////////////////////////////////////////////
/// @brief test the server-side MVCC behaviour
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
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Max Neunhoeffer
/// @author Copyright 2015, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

var jsunity = require("jsunity");
var internal = require("internal");
var db = internal.db;
var print = internal.print;

// -----------------------------------------------------------------------------
// --SECTION--                                                  database methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief test suite: dropping databases while holding references
////////////////////////////////////////////////////////////////////////////////

describe("MVCC", function () {
  'use strict';

  var cn = "UnitTestsMvcc";

  function verifyVisibilityForTransaction (t, key, value, visibility) {
    var c = db[cn];
    if (t !== null) {
      db._pushTransaction(t.id);
    }
    var d;
    var error;
    try {
      d = c.mvccDocument(key);
      error = {};
    }
    catch (e) {
      d = {};
      error = e;
    }
    if (visibility === "VISIBLE") {
      expect(d._key).toEqual(key);
      expect(d.Hallo).toEqual(value);
    }
    else if (visibility === "INVISIBLE") {
      expect(d).toEqual({});
      expect(error.errorNum).toEqual(1202);
    }
    if (t != null) {
      db._popTransaction(t.id);
    }
  }

  function verifyTransactionStack (stack) {
    expect(db._stackTransactions().map(function(t) { return t.id; }))
       .toEqual(stack.map(function(t) { return t.id; }));
  }

  beforeEach(function () {
    db._useDatabase("_system");
    db._drop(cn);
    db._create(cn);
  });

  afterEach(function () {
    // Cleanup any mess that could have been left:
    while (true) {
      try {
        db._popTransaction();
      }
      catch (e) {
        break;
      }
    }
    db._drop(cn);
  });

////////////////////////////////////////////////////////////////////////////////
/// @brief test implicitly committed transactions
////////////////////////////////////////////////////////////////////////////////

  it("should MVCC implicit transactions correctly", function () {
    var c = db[cn];
    verifyTransactionStack([]);

    // Create a document, implicit transactions:
    c.mvccInsert({_key: "doc1", Hallo: 1});
    verifyTransactionStack([]);
    verifyVisibilityForTransaction(null, "doc1", 1, "VISIBLE");

    // Replace a document, implicit transactions:
    c.mvccReplace("doc1", {Hallo: 2});
    verifyTransactionStack([]);
    verifyVisibilityForTransaction(null, "doc1", 2, "VISIBLE");
    
    // Update a document, implicit transactions:
    c.mvccUpdate("doc1", {Hallo: 3});
    verifyTransactionStack([]);
    verifyVisibilityForTransaction(null, "doc1", 3, "VISIBLE");
    
    // Try to find a non-existing document:
    verifyTransactionStack([]);
    verifyVisibilityForTransaction(null, "doc2", null, "INVISIBLE");

    // Remove a document, implicit transactions:
    expect(c.mvccRemove("doc1")).toEqual(true);

    // See if it is gone:
    verifyTransactionStack([]);
    verifyVisibilityForTransaction(null, "doc1", null, "INVISIBLE");

    expect(db._stackTransactions()).toEqual([]);
  });
      
////////////////////////////////////////////////////////////////////////////////
/// @brief test explicitly committed transactions
////////////////////////////////////////////////////////////////////////////////

  it("should MVCC explicit transactions correctly ",
     function () {
    var c = db[cn];
    verifyTransactionStack([]);

    // Create a document, explicit transaction:
    var trx = db._beginTransaction();
    c.mvccInsert({_key: "doc1", Hallo: 1});
    // Read within the same transaction:
    verifyTransactionStack([trx]);
    verifyVisibilityForTransaction(null, "doc1", 1, "VISIBLE");

    db._commitTransaction(trx.id);
    
    // Read with an implicit transaction:
    verifyTransactionStack([]);
    verifyVisibilityForTransaction(null, "doc1", 1, "VISIBLE");

    // Read with an explicit transaction:
    trx = db._beginTransaction();
    verifyTransactionStack([trx]);
    verifyVisibilityForTransaction(null, "doc1", 1, "VISIBLE");
    db._commitTransaction(trx.id);
    verifyTransactionStack([]);

    // Replace a document, explicit transaction:
    trx = db._beginTransaction();
    c.mvccReplace("doc1", {Hallo: 2});
    // Read in the same transaction:
    verifyTransactionStack([trx]);
    verifyVisibilityForTransaction(null, "doc1", 2, "VISIBLE");

    db._commitTransaction(trx.id);
    
    // Read with an implicit transaction:
    verifyTransactionStack([]);
    verifyVisibilityForTransaction(null, "doc1", 2, "VISIBLE");
    
    // Read with an explicit transaction:
    trx = db._beginTransaction();
    verifyTransactionStack([trx]);
    verifyVisibilityForTransaction(null, "doc1", 2, "VISIBLE");
    db._commitTransaction(trx.id);

    // Update a document, explicit transaction:
    trx = db._beginTransaction();
    c.mvccUpdate("doc1", {Hallo: 3});

    // Read within same transaction:
    verifyTransactionStack([trx]);
    verifyVisibilityForTransaction(null, "doc1", 3, "VISIBLE");
    db._commitTransaction(trx.id);
    
    // Read with an implicit transaction:
    verifyTransactionStack([]);
    verifyVisibilityForTransaction(null, "doc1", 3, "VISIBLE");
    
    // Read with an explicit transaction:
    trx = db._beginTransaction();
    verifyTransactionStack([trx]);
    verifyVisibilityForTransaction(null, "doc1", 3, "VISIBLE");
    db._commitTransaction(trx.id);

    // Try to find a non-existing document, explicit transaction:
    trx = db._beginTransaction();
    verifyTransactionStack([trx]);
    verifyVisibilityForTransaction(null, "doc2", null, "INVISIBLE");
    db._commitTransaction(trx.id);

    // Remove a document, explicit transactions:
    trx = db._beginTransaction();
    expect(c.mvccRemove("doc1")).toEqual(true);
    verifyTransactionStack([trx]);
    verifyVisibilityForTransaction(null, "doc1", null, "INVISIBLE");
    db._commitTransaction(trx.id);
    verifyTransactionStack([]);
    verifyVisibilityForTransaction(null, "doc1", null, "INVISIBLE");
  });

////////////////////////////////////////////////////////////////////////////////
/// @brief test explicitly aborted transactions
////////////////////////////////////////////////////////////////////////////////

  it("should MVCC explicitly aborted transactions correctly ",
     function () {
    var c = db[cn];
    verifyTransactionStack([]);

    // Create a document, explicit transaction:
    var trx = db._beginTransaction();
    c.mvccInsert({_key: "doc1", Hallo: 1});
    // Read within the same transaction:
    verifyTransactionStack([trx]);
    verifyVisibilityForTransaction(null, "doc1", 1, "VISIBLE");

    db._rollbackTransaction(trx.id);
    
    // Read with an implicit transaction:
    verifyTransactionStack([]);
    verifyVisibilityForTransaction(null, "doc1", null, "INVISIBLE");

    // Read with an explicit transaction:
    trx = db._beginTransaction();
    verifyTransactionStack([trx]);
    verifyVisibilityForTransaction(null, "doc1", null, "INVISIBLE");
    db._commitTransaction(trx.id);
    verifyTransactionStack([]);

    // Create it after all:
    c.mvccInsert({_key: "doc1", Hallo: 1});

    // Replace a document, explicit transaction:
    trx = db._beginTransaction();
    c.mvccReplace("doc1", {Hallo: 2});
    // Read in the same transaction:
    verifyTransactionStack([trx]);
    verifyVisibilityForTransaction(null, "doc1", 2, "VISIBLE");

    db._rollbackTransaction(trx.id);
    
    // Read with an implicit transaction:
    verifyTransactionStack([]);
    verifyVisibilityForTransaction(null, "doc1", 1, "VISIBLE");
    
    // Read with an explicit transaction:
    trx = db._beginTransaction();
    verifyTransactionStack([trx]);
    verifyVisibilityForTransaction(null, "doc1", 1, "VISIBLE");
    db._commitTransaction(trx.id);

    // Now actuall replace it:
    c.mvccReplace("doc1", {Hallo: 2});

    // Update a document, explicit transaction:
    trx = db._beginTransaction();
    c.mvccUpdate("doc1", {Hallo: 3});

    // Read within same transaction:
    verifyTransactionStack([trx]);
    verifyVisibilityForTransaction(null, "doc1", 3, "VISIBLE");

    db._rollbackTransaction(trx.id);
    
    // Read with an implicit transaction:
    verifyTransactionStack([]);
    verifyVisibilityForTransaction(null, "doc1", 2, "VISIBLE");
    
    // Read with an explicit transaction:
    trx = db._beginTransaction();
    verifyTransactionStack([trx]);
    verifyVisibilityForTransaction(null, "doc1", 2, "VISIBLE");
    db._commitTransaction(trx.id);

    // Now really update:
    c.mvccUpdate("doc1", {Hallo: 3});

    // Try to find a non-existing document, explicit transaction:
    trx = db._beginTransaction();
    verifyTransactionStack([trx]);
    verifyVisibilityForTransaction(null, "doc2", null, "INVISIBLE");
    db._commitTransaction(trx.id);

    // Remove a document, explicit transactions:
    trx = db._beginTransaction();
    expect(c.mvccRemove("doc1")).toEqual(true);
    verifyTransactionStack([trx]);
    verifyVisibilityForTransaction(null, "doc1", null, "INVISIBLE");
    db._rollbackTransaction(trx.id);
    verifyTransactionStack([]);
    verifyVisibilityForTransaction(null, "doc1", 3, "VISIBLE");

    // Now really remove it:
    expect(c.mvccRemove("doc1")).toEqual(true);
  });

////////////////////////////////////////////////////////////////////////////////
/// @brief test a second observing top level transaction
////////////////////////////////////////////////////////////////////////////////

  it("should MVCC separate observing top level transactions correctly",
     function () {
    var c = db[cn];
    var d;
    var t1,     // started before trx
        t2,     // started after trx but before operation
        t3,     // started after operation but during trx
        t4;     // started after trx committed

    verifyTransactionStack([]);

    t1 = db._beginTransaction({top: true}); db._popTransaction();

    verifyTransactionStack([]);

    // Create a document, explicit transaction:
    var trx = db._beginTransaction();

    verifyTransactionStack([trx]);
    t2 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([trx]);

    c.mvccInsert({_key: "doc1", Hallo: 1});
    // Read within the same transaction:
    verifyTransactionStack([trx]);
    verifyVisibilityForTransaction(null, "doc1", 1, "VISIBLE");

    t3 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([trx]);

    verifyVisibilityForTransaction(t1, "doc1", null, "INVISIBLE");
    verifyVisibilityForTransaction(t2, "doc1", null, "INVISIBLE");
    verifyVisibilityForTransaction(t3, "doc1", null, "INVISIBLE");

    db._commitTransaction(trx.id);
    verifyTransactionStack([]);

    t4 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([]);

    verifyVisibilityForTransaction(t1, "doc1", null, "INVISIBLE");
    verifyVisibilityForTransaction(t2, "doc1", null, "INVISIBLE");
    verifyVisibilityForTransaction(t3, "doc1", null, "INVISIBLE");
    verifyVisibilityForTransaction(t4, "doc1", 1, "VISIBLE");

    db._pushTransaction(t4.id); db._rollbackTransaction(t4.id);
    db._pushTransaction(t3.id); db._rollbackTransaction(t3.id);
    db._pushTransaction(t2.id); db._rollbackTransaction(t2.id);
    db._pushTransaction(t1.id); db._rollbackTransaction(t1.id);

    // Replace a document, explicit transaction:

    t1 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([]);

    trx = db._beginTransaction();
    verifyTransactionStack([trx]);

    t2 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([trx]);

    verifyVisibilityForTransaction(t1, "doc1", 1, "VISIBLE");
    verifyVisibilityForTransaction(t2, "doc1", 1, "VISIBLE");

    c.mvccReplace("doc1", {Hallo: 2});
    t3 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([trx]);

    verifyVisibilityForTransaction(t1, "doc1", 1, "VISIBLE");
    verifyVisibilityForTransaction(t2, "doc1", 1, "VISIBLE");
    verifyVisibilityForTransaction(t3, "doc1", 1, "VISIBLE");

    // Read in the same transaction:
    verifyTransactionStack([trx]);
    verifyVisibilityForTransaction(null, "doc1", 2, "VISIBLE");

    db._commitTransaction(trx.id);
    verifyTransactionStack([]);
    t4 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([]);
    
    verifyVisibilityForTransaction(t1, "doc1", 1, "VISIBLE");
    verifyVisibilityForTransaction(t2, "doc1", 1, "VISIBLE");
    verifyVisibilityForTransaction(t3, "doc1", 1, "VISIBLE");
    verifyVisibilityForTransaction(t4, "doc1", 2, "VISIBLE");

    db._pushTransaction(t4.id); db._rollbackTransaction(t4.id);
    db._pushTransaction(t3.id); db._rollbackTransaction(t3.id);
    db._pushTransaction(t2.id); db._rollbackTransaction(t2.id);
    db._pushTransaction(t1.id); db._rollbackTransaction(t1.id);

    // Update a document, explicit transaction:

    t1 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([]);

    trx = db._beginTransaction();
    verifyTransactionStack([trx]);

    t2 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([trx]);

    verifyVisibilityForTransaction(t1, "doc1", 2, "VISIBLE");
    verifyVisibilityForTransaction(t2, "doc1", 2, "VISIBLE");

    c.mvccUpdate("doc1", {Hallo: 3});
    t3 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([trx]);

    verifyVisibilityForTransaction(t1, "doc1", 2, "VISIBLE");
    verifyVisibilityForTransaction(t2, "doc1", 2, "VISIBLE");
    verifyVisibilityForTransaction(t3, "doc1", 2, "VISIBLE");

    // Read in the same transaction:
    d = c.mvccDocument("doc1");
    verifyTransactionStack([trx]);
    verifyVisibilityForTransaction(null, "doc1", 3, "VISIBLE");
    
    db._commitTransaction(trx.id);
    verifyTransactionStack([]);

    t4 = db._beginTransaction({top: true}); db._popTransaction();

    verifyVisibilityForTransaction(t1, "doc1", 2, "VISIBLE");
    verifyVisibilityForTransaction(t2, "doc1", 2, "VISIBLE");
    verifyVisibilityForTransaction(t3, "doc1", 2, "VISIBLE");
    verifyVisibilityForTransaction(t4, "doc1", 3, "VISIBLE");

    db._pushTransaction(t4.id); db._rollbackTransaction(t4.id);
    db._pushTransaction(t3.id); db._rollbackTransaction(t3.id);
    db._pushTransaction(t2.id); db._rollbackTransaction(t2.id);
    db._pushTransaction(t1.id); db._rollbackTransaction(t1.id);

    // Remove a document, explicit transactions:
    t1 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([]);

    trx = db._beginTransaction();
    verifyTransactionStack([trx]);

    t2 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([trx]);

    verifyVisibilityForTransaction(t1, "doc1", 3, "VISIBLE");
    verifyVisibilityForTransaction(t2, "doc1", 3, "VISIBLE");

    expect(c.mvccRemove("doc1")).toEqual(true);

    t3 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([trx]);

    verifyVisibilityForTransaction(t1, "doc1", 3, "VISIBLE");
    verifyVisibilityForTransaction(t2, "doc1", 3, "VISIBLE");
    verifyVisibilityForTransaction(t3, "doc1", 3, "VISIBLE");

    db._commitTransaction(trx.id);
    verifyTransactionStack([]);

    t4 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([]);

    verifyVisibilityForTransaction(t1, "doc1", 3, "VISIBLE");
    verifyVisibilityForTransaction(t2, "doc1", 3, "VISIBLE");
    verifyVisibilityForTransaction(t3, "doc1", 3, "VISIBLE");
    verifyVisibilityForTransaction(t4, "doc1", null, "INVISIBLE");

    db._pushTransaction(t4.id); db._rollbackTransaction(t4.id);
    db._pushTransaction(t3.id); db._rollbackTransaction(t3.id);
    db._pushTransaction(t2.id); db._rollbackTransaction(t2.id);
    db._pushTransaction(t1.id); db._rollbackTransaction(t1.id);

    verifyTransactionStack([]);
  });

////////////////////////////////////////////////////////////////////////////////
/// @brief test a second observing top level transaction
////////////////////////////////////////////////////////////////////////////////

  it("should MVCC separate observing top level transactions correctly (abort)",
     function () {
    var c = db[cn];
    var d;
    var t1,     // started before trx
        t2,     // started after trx but before operation
        t3,     // started after operation but during trx
        t4;     // started after trx committed

    verifyTransactionStack([]);

    t1 = db._beginTransaction({top: true}); db._popTransaction();

    verifyTransactionStack([]);

    // Create a document, explicit transaction:
    var trx = db._beginTransaction();

    verifyTransactionStack([trx]);
    t2 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([trx]);

    c.mvccInsert({_key: "doc1", Hallo: 1});
    // Read within the same transaction:
    verifyTransactionStack([trx]);
    verifyVisibilityForTransaction(null, "doc1", 1, "VISIBLE");

    t3 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([trx]);

    verifyVisibilityForTransaction(t1, "doc1", null, "INVISIBLE");
    verifyVisibilityForTransaction(t2, "doc1", null, "INVISIBLE");
    verifyVisibilityForTransaction(t3, "doc1", null, "INVISIBLE");

    db._rollbackTransaction(trx.id);
    verifyTransactionStack([]);

    t4 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([]);

    verifyVisibilityForTransaction(t1, "doc1", null, "INVISIBLE");
    verifyVisibilityForTransaction(t2, "doc1", null, "INVISIBLE");
    verifyVisibilityForTransaction(t3, "doc1", null, "INVISIBLE");
    verifyVisibilityForTransaction(t4, "doc1", null, "INVISIBLE");

    db._pushTransaction(t4.id); db._rollbackTransaction(t4.id);
    db._pushTransaction(t3.id); db._rollbackTransaction(t3.id);
    db._pushTransaction(t2.id); db._rollbackTransaction(t2.id);
    db._pushTransaction(t1.id); db._rollbackTransaction(t1.id);

    // Really insert it after all:
    c.mvccInsert({_key: "doc1", Hallo: 1});

    // Replace a document, explicit transaction:

    t1 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([]);

    trx = db._beginTransaction();
    verifyTransactionStack([trx]);

    t2 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([trx]);

    verifyVisibilityForTransaction(t1, "doc1", 1, "VISIBLE");
    verifyVisibilityForTransaction(t2, "doc1", 1, "VISIBLE");

    c.mvccReplace("doc1", {Hallo: 2});
    t3 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([trx]);

    verifyVisibilityForTransaction(t1, "doc1", 1, "VISIBLE");
    verifyVisibilityForTransaction(t2, "doc1", 1, "VISIBLE");
    verifyVisibilityForTransaction(t3, "doc1", 1, "VISIBLE");

    // Read in the same transaction:
    verifyTransactionStack([trx]);
    verifyVisibilityForTransaction(null, "doc1", 2, "VISIBLE");

    db._rollbackTransaction(trx.id);
    verifyTransactionStack([]);
    t4 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([]);
    
    verifyVisibilityForTransaction(t1, "doc1", 1, "VISIBLE");
    verifyVisibilityForTransaction(t2, "doc1", 1, "VISIBLE");
    verifyVisibilityForTransaction(t3, "doc1", 1, "VISIBLE");
    verifyVisibilityForTransaction(t4, "doc1", 1, "VISIBLE");

    db._pushTransaction(t4.id); db._rollbackTransaction(t4.id);
    db._pushTransaction(t3.id); db._rollbackTransaction(t3.id);
    db._pushTransaction(t2.id); db._rollbackTransaction(t2.id);
    db._pushTransaction(t1.id); db._rollbackTransaction(t1.id);

    // Replace it after all:
    c.mvccReplace("doc1", {Hallo: 2});

    // Update a document, explicit transaction:

    t1 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([]);

    trx = db._beginTransaction();
    verifyTransactionStack([trx]);

    t2 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([trx]);

    verifyVisibilityForTransaction(t1, "doc1", 2, "VISIBLE");
    verifyVisibilityForTransaction(t2, "doc1", 2, "VISIBLE");

    c.mvccUpdate("doc1", {Hallo: 3});
    t3 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([trx]);

    verifyVisibilityForTransaction(t1, "doc1", 2, "VISIBLE");
    verifyVisibilityForTransaction(t2, "doc1", 2, "VISIBLE");
    verifyVisibilityForTransaction(t3, "doc1", 2, "VISIBLE");

    // Read in the same transaction:
    d = c.mvccDocument("doc1");
    verifyTransactionStack([trx]);
    verifyVisibilityForTransaction(null, "doc1", 3, "VISIBLE");
    
    db._rollbackTransaction(trx.id);
    verifyTransactionStack([]);

    t4 = db._beginTransaction({top: true}); db._popTransaction();

    verifyVisibilityForTransaction(t1, "doc1", 2, "VISIBLE");
    verifyVisibilityForTransaction(t2, "doc1", 2, "VISIBLE");
    verifyVisibilityForTransaction(t3, "doc1", 2, "VISIBLE");
    verifyVisibilityForTransaction(t4, "doc1", 2, "VISIBLE");

    db._pushTransaction(t4.id); db._rollbackTransaction(t4.id);
    db._pushTransaction(t3.id); db._rollbackTransaction(t3.id);
    db._pushTransaction(t2.id); db._rollbackTransaction(t2.id);
    db._pushTransaction(t1.id); db._rollbackTransaction(t1.id);

    // Update it after all:
    c.mvccUpdate("doc1", {Hallo: 3});

    // Remove a document, explicit transactions:
    t1 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([]);

    trx = db._beginTransaction();
    verifyTransactionStack([trx]);

    t2 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([trx]);

    verifyVisibilityForTransaction(t1, "doc1", 3, "VISIBLE");
    verifyVisibilityForTransaction(t2, "doc1", 3, "VISIBLE");

    expect(c.mvccRemove("doc1")).toEqual(true);

    t3 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([trx]);

    verifyVisibilityForTransaction(t1, "doc1", 3, "VISIBLE");
    verifyVisibilityForTransaction(t2, "doc1", 3, "VISIBLE");
    verifyVisibilityForTransaction(t3, "doc1", 3, "VISIBLE");

    db._rollbackTransaction(trx.id);
    verifyTransactionStack([]);

    t4 = db._beginTransaction({top: true}); db._popTransaction();
    verifyTransactionStack([]);

    verifyVisibilityForTransaction(t1, "doc1", 3, "VISIBLE");
    verifyVisibilityForTransaction(t2, "doc1", 3, "VISIBLE");
    verifyVisibilityForTransaction(t3, "doc1", 3, "VISIBLE");
    verifyVisibilityForTransaction(t4, "doc1", 3, "VISIBLE");

    db._pushTransaction(t4.id); db._rollbackTransaction(t4.id);
    db._pushTransaction(t3.id); db._rollbackTransaction(t3.id);
    db._pushTransaction(t2.id); db._rollbackTransaction(t2.id);
    db._pushTransaction(t1.id); db._rollbackTransaction(t1.id);

    verifyTransactionStack([]);

    // Now finally really remove it:
    expect(c.mvccRemove("doc1")).toEqual(true);
    verifyTransactionStack([]);
  });

////////////////////////////////////////////////////////////////////////////////
/// @brief test explicitly committed subtransaction from the perspective
/// of the parent transaction.
////////////////////////////////////////////////////////////////////////////////

  it("should MVCC explicit subtransactions correctly from the parent perspective",
     function () {
    var c = db[cn];
    verifyTransactionStack([]);

    var par = db._beginTransaction();
    verifyTransactionStack([par]);

    // Create a document, explicit transaction:
    var trx = db._beginTransaction();
    c.mvccInsert({_key: "doc1", Hallo: 1});
    // Read within the same transaction:
    verifyTransactionStack([par, trx]);
    verifyVisibilityForTransaction(null, "doc1", 1, "VISIBLE");
    // Now let the parent have a look:
    db._popTransaction(trx.id);
    verifyTransactionStack([par]);
    verifyVisibilityForTransaction(null, "doc1", null, "INVISIBLE");
    db._pushTransaction(trx.id);
    verifyTransactionStack([par, trx]);

    db._commitTransaction(trx.id);
    
    // Read with in the context of the parent:
    verifyTransactionStack([par]);
    verifyVisibilityForTransaction(null, "doc1", 1, "VISIBLE");

    // Read with an explicit subtransaction:
    trx = db._beginTransaction();
    verifyTransactionStack([par, trx]);
    verifyVisibilityForTransaction(null, "doc1", 1, "VISIBLE");
    db._commitTransaction(trx.id);
    verifyTransactionStack([par]);

    // Replace a document, explicit subtransaction:
    trx = db._beginTransaction();
    c.mvccReplace("doc1", {Hallo: 2});
    // Read in the same transaction:
    verifyTransactionStack([par, trx]);
    verifyVisibilityForTransaction(null, "doc1", 2, "VISIBLE");
    // Now let the parent have a look:
    db._popTransaction(trx.id);
    verifyTransactionStack([par]);
    verifyVisibilityForTransaction(null, "doc1", 1, "VISIBLE");
    db._pushTransaction(trx.id);
    verifyTransactionStack([par, trx]);

    db._commitTransaction(trx.id);
    
    // Read in the context of the parent:
    verifyTransactionStack([par]);
    verifyVisibilityForTransaction(null, "doc1", 2, "VISIBLE");
    
    // Read with an explicit subtransaction:
    trx = db._beginTransaction();
    verifyTransactionStack([par, trx]);
    verifyVisibilityForTransaction(null, "doc1", 2, "VISIBLE");
    db._commitTransaction(trx.id);
    verifyTransactionStack([par]);

    // Update a document, explicit transaction:
    trx = db._beginTransaction();
    verifyTransactionStack([par, trx]);
    c.mvccUpdate("doc1", {Hallo: 3});

    // Read within same transaction:
    verifyTransactionStack([par, trx]);
    verifyVisibilityForTransaction(null, "doc1", 3, "VISIBLE");
    // Now let the parent have a look:
    db._popTransaction(trx.id);
    verifyTransactionStack([par]);
    verifyVisibilityForTransaction(null, "doc1", 2, "VISIBLE");
    db._pushTransaction(trx.id);
    verifyTransactionStack([par, trx]);

    db._commitTransaction(trx.id);
    
    // Read in the context of the parent:
    verifyTransactionStack([par]);
    verifyVisibilityForTransaction(null, "doc1", 3, "VISIBLE");
    
    // Read with an explicit subtransaction:
    trx = db._beginTransaction();
    verifyTransactionStack([par, trx]);
    verifyVisibilityForTransaction(null, "doc1", 3, "VISIBLE");
    db._commitTransaction(trx.id);
    verifyTransactionStack([par]);

    // Try to find a non-existing document, explicit transaction:
    trx = db._beginTransaction();
    verifyTransactionStack([par, trx]);
    verifyVisibilityForTransaction(null, "doc2", null, "INVISIBLE");
    db._commitTransaction(trx.id);
    verifyTransactionStack([par]);

    // Remove a document, explicit subtransactions:
    trx = db._beginTransaction();
    expect(c.mvccRemove("doc1")).toEqual(true);
    verifyTransactionStack([par, trx]);
    verifyVisibilityForTransaction(null, "doc1", null, "INVISIBLE");
    // Now let the parent have a look:
    db._popTransaction(trx.id);
    verifyTransactionStack([par]);
    verifyVisibilityForTransaction(null, "doc1", 3, "VISIBLE");
    db._pushTransaction(trx.id);
    verifyTransactionStack([par, trx]);

    db._commitTransaction(trx.id);
    verifyTransactionStack([par]);

    // Check in the context of the parent:
    verifyVisibilityForTransaction(null, "doc1", null, "INVISIBLE");

    // Read with an explicit subtransaction:
    trx = db._beginTransaction();
    verifyTransactionStack([par, trx]);
    verifyVisibilityForTransaction(null, "doc1", null, "INVISIBLE");
    db._commitTransaction(trx.id);
    verifyTransactionStack([par]);

    // End main transaction:
    db._commitTransaction(par.id);
    verifyTransactionStack([]);
  });

////////////////////////////////////////////////////////////////////////////////
/// @brief test explicitly aborted subtransaction from the perspective
/// of the parent transaction.
////////////////////////////////////////////////////////////////////////////////

  it("should MVCC explicit aborted subtransactions correctly from the parent perspective",
     function () {
    var c = db[cn];
    verifyTransactionStack([]);

    var par = db._beginTransaction();
    verifyTransactionStack([par]);

    // Create a document, explicit transaction:
    var trx = db._beginTransaction();
    c.mvccInsert({_key: "doc1", Hallo: 1});
    // Read within the same transaction:
    verifyTransactionStack([par, trx]);
    verifyVisibilityForTransaction(null, "doc1", 1, "VISIBLE");
    // Now let the parent have a look:
    db._popTransaction(trx.id);
    verifyTransactionStack([par]);
    verifyVisibilityForTransaction(null, "doc1", null, "INVISIBLE");
    db._pushTransaction(trx.id);
    verifyTransactionStack([par, trx]);

    db._rollbackTransaction(trx.id);
    
    // Read with in the context of the parent:
    verifyTransactionStack([par]);
    verifyVisibilityForTransaction(null, "doc1", null, "INVISIBLE");

    // Read with an explicit subtransaction:
    trx = db._beginTransaction();
    verifyTransactionStack([par, trx]);
    verifyVisibilityForTransaction(null, "doc1", null, "INVISIBLE");
    db._commitTransaction(trx.id);
    verifyTransactionStack([par]);

    // Not really insert the document:
    c.mvccInsert({_key: "doc1", Hallo: 1});

    // Replace a document, explicit subtransaction:
    trx = db._beginTransaction();
    c.mvccReplace("doc1", {Hallo: 2});
    // Read in the same transaction:
    verifyTransactionStack([par, trx]);
    verifyVisibilityForTransaction(null, "doc1", 2, "VISIBLE");
    // Now let the parent have a look:
    db._popTransaction(trx.id);
    verifyTransactionStack([par]);
    verifyVisibilityForTransaction(null, "doc1", 1, "VISIBLE");
    db._pushTransaction(trx.id);
    verifyTransactionStack([par, trx]);

    db._rollbackTransaction(trx.id);
    
    // Read in the context of the parent:
    verifyTransactionStack([par]);
    verifyVisibilityForTransaction(null, "doc1", 1, "VISIBLE");
    
    // Read with an explicit subtransaction:
    trx = db._beginTransaction();
    verifyTransactionStack([par, trx]);
    verifyVisibilityForTransaction(null, "doc1", 1, "VISIBLE");
    db._commitTransaction(trx.id);
    verifyTransactionStack([par]);

    // Now really replace it:
    c.mvccReplace("doc1", {Hallo: 2});

    // Update a document, explicit transaction:
    trx = db._beginTransaction();
    verifyTransactionStack([par, trx]);
    c.mvccUpdate("doc1", {Hallo: 3});

    // Read within same transaction:
    verifyTransactionStack([par, trx]);
    verifyVisibilityForTransaction(null, "doc1", 3, "VISIBLE");
    // Now let the parent have a look:
    db._popTransaction(trx.id);
    verifyTransactionStack([par]);
    verifyVisibilityForTransaction(null, "doc1", 2, "VISIBLE");
    db._pushTransaction(trx.id);
    verifyTransactionStack([par, trx]);

    db._rollbackTransaction(trx.id);
    
    // Read in the context of the parent:
    verifyTransactionStack([par]);
    verifyVisibilityForTransaction(null, "doc1", 2, "VISIBLE");
    
    // Read with an explicit subtransaction:
    trx = db._beginTransaction();
    verifyTransactionStack([par, trx]);
    verifyVisibilityForTransaction(null, "doc1", 2, "VISIBLE");
    db._commitTransaction(trx.id);
    verifyTransactionStack([par]);

    // Now really update the document:
    c.mvccUpdate("doc1", {Hallo: 3});

    // Remove a document, explicit subtransactions:
    trx = db._beginTransaction();
    expect(c.mvccRemove("doc1")).toEqual(true);
    verifyTransactionStack([par, trx]);
    verifyVisibilityForTransaction(null, "doc1", null, "INVISIBLE");
    // Now let the parent have a look:
    db._popTransaction(trx.id);
    verifyTransactionStack([par]);
    verifyVisibilityForTransaction(null, "doc1", 3, "VISIBLE");
    db._pushTransaction(trx.id);
    verifyTransactionStack([par, trx]);

    db._rollbackTransaction(trx.id);
    verifyTransactionStack([par]);

    // Check in the context of the parent:
    verifyVisibilityForTransaction(null, "doc1", 3, "VISIBLE");

    // Read with an explicit subtransaction:
    trx = db._beginTransaction();
    verifyTransactionStack([par, trx]);
    verifyVisibilityForTransaction(null, "doc1", 3, "VISIBLE");
    db._commitTransaction(trx.id);
    verifyTransactionStack([par]);

    // Now really remove it:
    expect(c.mvccRemove("doc1")).toEqual(true);

    // End main transaction:
    db._commitTransaction(par.id);
    verifyTransactionStack([]);
  });

});

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// @addtogroup\\|// --SECTION--\\|/// @page\\|/// @}\\)"
// End:

