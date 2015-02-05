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

  function verifyDocumentNotFound (key) {
    var c = db[cn];

    try {
      var d = c.mvccDocument(key);
      expect("should throw").toEqual("document not found");
    } catch (e) {
      expect(e.errorNum).toEqual(1202);
    }
  }

  beforeEach(function () {
    db._useDatabase("_system");
    db._drop(cn);
    db._create(cn);
  });

  afterEach(function () {
    db._drop(cn);
  });

////////////////////////////////////////////////////////////////////////////////
/// @brief test implicitly committed transactions
////////////////////////////////////////////////////////////////////////////////

  it("should correctly implement logic for implicit transactions", function () {
    var c = db[cn];
    var d;

    expect(db._stackTransactions()).toEqual([]);

    // Create a document, implicit transactions:
    c.mvccInsert({_key: "doc1", Hallo: 1});
    d = c.mvccDocument("doc1");
    expect(d._key).toEqual("doc1");
    expect(d.Hallo).toEqual(1);

    // Replace a document, implicit transactions:
    c.mvccReplace("doc1", {Hallo: 2});
    d = c.mvccDocument("doc1");
    expect(d._key).toEqual("doc1");
    expect(d.Hallo).toEqual(2);
    
    // Update a document, implicit transactions:
    c.mvccUpdate("doc1", {Hallo: 3});
    d = c.mvccDocument("doc1");
    expect(d._key).toEqual("doc1");
    expect(d.Hallo).toEqual(3);
    
    // Try to find a non-existing document:
    verifyDocumentNotFound("doc2");
    try {
      d = c.mvccDocument("doc2");
      expect("should throw").toEqual("document not found");
    } catch (e1) {
      expect(e1.errorNum).toEqual(1202);
    }

    // Remove a document, implicit transactions:
    expect(c.mvccRemove("doc1")).toEqual(true);
    expect(db._stackTransactions()).toEqual([]);
  });
      
////////////////////////////////////////////////////////////////////////////////
/// @brief test implicitly committed transactions
////////////////////////////////////////////////////////////////////////////////

  it("should implement the MVCC logic correctly for explicit transactions",
     function () {
    var c = db[cn];
    var d;

    expect(db._stackTransactions()).toEqual([]);

    // Create a document, explicit transaction:
    var trx = db._beginTransaction();
    c.mvccInsert({_key: "doc1", Hallo: 1});
    // Read within the same transaction:
    d = c.mvccDocument("doc1");
    expect(d._key).toEqual("doc1");
    expect(d.Hallo).toEqual(1);
    db._commitTransaction(trx.id);

    expect(db._stackTransactions()).toEqual([]);
    
    // Read with an implicit transaction:
    d = c.mvccDocument("doc1");
    expect(d._key).toEqual("doc1");
    expect(d.Hallo).toEqual(1);

    // Read with an explicit transaction:
    trx = db._beginTransaction();
    d = c.mvccDocument("doc1");
    db._commitTransaction(trx.id);
    expect(d._key).toEqual("doc1");
    expect(d.Hallo).toEqual(1);

    // Replace a document, explicit transaction:
    trx = db._beginTransaction();
    c.mvccReplace("doc1", {Hallo: 2});
    // Read in the same transaction:
    d = c.mvccDocument("doc1");
    expect(d._key).toEqual("doc1");
    expect(d.Hallo).toEqual(2);
    db._commitTransaction(trx.id);
    
    expect(db._stackTransactions()).toEqual([]);
    
    // Read with an implicit transaction:
    d = c.mvccDocument("doc1");
    expect(d._key).toEqual("doc1");
    expect(d.Hallo).toEqual(2);
    
    // Read with an explicit transaction:
    trx = db._beginTransaction();
    d = c.mvccDocument("doc1");
    db._commitTransaction(trx.id);
    expect(d._key).toEqual("doc1");
    expect(d.Hallo).toEqual(2);

    // Update a document, explicit transaction:
    trx = db._beginTransaction();
    c.mvccUpdate("doc1", {Hallo: 3});
    d = c.mvccDocument("doc1");
    expect(d._key).toEqual("doc1");
    expect(d.Hallo).toEqual(3);
    db._commitTransaction(trx.id);
    
    // Read with an implicit transaction:
    d = c.mvccDocument("doc1");
    expect(d._key).toEqual("doc1");
    expect(d.Hallo).toEqual(3);
    
    // Read with an explicit transaction:
    trx = db._beginTransaction();
    d = c.mvccDocument("doc1");
    db._commitTransaction(trx.id);
    expect(d._key).toEqual("doc1");
    expect(d.Hallo).toEqual(3);

    // Try to find a non-existing document, explicit transaction:
    try {
      trx = db._beginTransaction();
      d = c.mvccDocument("doc2");
      expect("to throw").toEqual("document not found");
    } catch (e1) {
      db._commitTransaction(trx.id);
      expect(e1.errorNum).toEqual(1202);
    }

    // Remove a document, explicit transactions:
    trx = db._beginTransaction();
    expect(c.mvccRemove("doc1")).toEqual(true);
    db._commitTransaction(trx.id);
  });

////////////////////////////////////////////////////////////////////////////////
/// @brief test implicitly committed transactions
////////////////////////////////////////////////////////////////////////////////

  it("should implement the MVCC logic correctly for explicitly aborted transactions",
     function () {
    var c = db[cn];
    var d;

    expect(db._stackTransactions()).toEqual([]);

    // Create a document, explicit transaction:
    var trx = db._beginTransaction();
    c.mvccInsert({_key: "doc1", Hallo: 1});
    // Read within the same transaction:
    d = c.mvccDocument("doc1");
    expect(d._key).toEqual("doc1");
    expect(d.Hallo).toEqual(1);
    db._rollbackTransaction(trx.id);

    expect(db._stackTransactions()).toEqual([]);
    
    // Read with an implicit transaction:
    verifyDocumentNotFound("doc1");

    // Read with an explicit transaction:
    trx = db._beginTransaction();
    verifyDocumentNotFound("doc1");
    db._commitTransaction(trx.id);

    c.mvccInsert({_key: "doc1", Hallo: 1});

    // Replace a document, explicitly aborted transaction:
    trx = db._beginTransaction();
    c.mvccReplace("doc1", {Hallo: 2});
    // Read in the same transaction:
    d = c.mvccDocument("doc1");
    expect(d._key).toEqual("doc1");
    expect(d.Hallo).toEqual(2);
    db._rollbackTransaction(trx.id);
    
    expect(db._stackTransactions()).toEqual([]);
    
    // Read with an implicit transaction:
    d = c.mvccDocument("doc1");
    expect(d._key).toEqual("doc1");
    expect(d.Hallo).toEqual(1);
    
    // Read with an explicit transaction:
    trx = db._beginTransaction();
    d = c.mvccDocument("doc1");
    db._commitTransaction(trx.id);
    expect(d._key).toEqual("doc1");
    expect(d.Hallo).toEqual(1);

    // Update a document, explicitly aborted transaction:
    trx = db._beginTransaction();
    c.mvccUpdate("doc1", {Hallo: 3});
    d = c.mvccDocument("doc1");
    expect(d._key).toEqual("doc1");
    expect(d.Hallo).toEqual(3);
    db._rollbackTransaction(trx.id);
    
    // Read with an implicit transaction:
    d = c.mvccDocument("doc1");
    expect(d._key).toEqual("doc1");
    expect(d.Hallo).toEqual(1);
    
    // Read with an explicit transaction:
    trx = db._beginTransaction();
    d = c.mvccDocument("doc1");
    db._commitTransaction(trx.id);
    expect(d._key).toEqual("doc1");
    expect(d.Hallo).toEqual(1);

    // Remove a document, explicit transactions:
    trx = db._beginTransaction();
    expect(c.mvccRemove("doc1")).toEqual(true);
    db._rollbackTransaction(trx.id);

    // Read with an implicit transaction:
    d = c.mvccDocument("doc1");
    expect(d._key).toEqual("doc1");
    expect(d.Hallo).toEqual(1);
    
    // Read with an explicit transaction:
    trx = db._beginTransaction();
    d = c.mvccDocument("doc1");
    db._commitTransaction(trx.id);
    expect(d._key).toEqual("doc1");
    expect(d.Hallo).toEqual(1);

  });
});

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// @addtogroup\\|// --SECTION--\\|/// @page\\|/// @}\\)"
// End:

