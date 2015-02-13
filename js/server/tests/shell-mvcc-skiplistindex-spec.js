/*jshint strict: false */
/*global require, describe, expect, beforeEach, afterEach, it */
////////////////////////////////////////////////////////////////////////////////
/// @brief test the server-side MVCC behaviour for skip list indexes
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

var internal = require("internal");
var db = internal.db;

// -----------------------------------------------------------------------------
// --SECTION--                                                  database methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief test MVCC skiplist index non-sparse
////////////////////////////////////////////////////////////////////////////////

describe("MVCC skiplist index non-sparse", function () {
  'use strict';

  var cn = "UnitTestsMvccSkiplistIndex";

  beforeEach(function () {
    db._useDatabase("_system");
    db._drop(cn);
    var c = db._create(cn);
    c.ensureIndex({type:"skiplist", fields: ["value"], sparse: false, unique: false});
  });

  afterEach(function () {
    db._drop(cn);
  });

////////////////////////////////////////////////////////////////////////////////
/// @brief test non-sparse non-unique skiplist index w. low level query function
////////////////////////////////////////////////////////////////////////////////
  
  it("should do a non-sparse non-unique skiplist index correctly", function () {
    var c = db[cn];
    var idx = c.getIndexes()[1];

    // Empty:
    var r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r).toEqual([]);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r).toEqual([]);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r).toEqual([]);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r).toEqual([]);

    // One document:
    c.mvccInsert({value : 1});
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // Two documents:
    c.mvccInsert({value : 1});
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(2);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // Three documents, different values:
    c.mvccInsert({value : 2});
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(2);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // A thousand documents:
    var i;
    for (i = 0; i < 1000; i++) {
      c.mvccInsert({value : 1});
    }
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1002);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // A thousand more documents:
    for (i = 0; i < 1000; i++) {
      c.mvccInsert({value : i});
    }
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1003);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(2);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // Some documents with value: null
    c.mvccInsert({value : null});
    c.mvccInsert({value : null});
    c.mvccInsert({value : null});
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1003);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(2);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(3);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(3);

    // Some documents without a bound value
    c.mvccInsert({});
    c.mvccInsert({});
    c.mvccInsert({});
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1003);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(2);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(6);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(6);

    // Now delete again:
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    for (i = 0; i < r.length; i++) {
      c.mvccRemove(r[i]._key);
    }
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(2);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(6);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(6);

    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    for (i = 0; i < r.length; i++) {
      c.mvccRemove(r[i]._key);
    }
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(6);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(6);

    r = c.mvccByExampleSkiplist(idx, { value : null });
    for (i = 0; i < r.length; i++) {
      c.mvccRemove(r[i]._key);
    }
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);
  });
      
});

////////////////////////////////////////////////////////////////////////////////
/// @brief test MVCC skiplist index sparse
////////////////////////////////////////////////////////////////////////////////

describe("MVCC skiplist index sparse", function () {
  'use strict';

  var cn = "UnitTestsMvccHashIndex";

  beforeEach(function () {
    db._useDatabase("_system");
    db._drop(cn);
    var c = db._create(cn);
    c.ensureIndex({type:"skiplist", fields: ["value"], sparse: true, unique: false});
  });

  afterEach(function () {
    db._drop(cn);
  });

////////////////////////////////////////////////////////////////////////////////
/// @brief test non-sparse non-unique skiplist index and low level query function
////////////////////////////////////////////////////////////////////////////////
  
  it("should do a sparse non-unique skiplist index correctly", function () {
    var c = db[cn];
    var idx = c.getIndexes()[1];

    // Empty:
    var r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r).toEqual([]);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r).toEqual([]);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r).toEqual([]);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r).toEqual([]);

    // One document:
    c.mvccInsert({value : 1});
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // Two documents:
    c.mvccInsert({value : 1});
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(2);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // Three documents, different values:
    c.mvccInsert({value : 2});
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(2);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // A thousand documents:
    var i;
    for (i = 0; i < 1000; i++) {
      c.mvccInsert({value : 1});
    }
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1002);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // A thousand more documents:
    for (i = 0; i < 1000; i++) {
      c.mvccInsert({value : i});
    }
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1003);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(2);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // Some documents with value: null
    c.mvccInsert({value : null});
    c.mvccInsert({value : null});
    c.mvccInsert({value : null});
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1003);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(2);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // Some documents without a bound value
    c.mvccInsert({});
    c.mvccInsert({});
    c.mvccInsert({});
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1003);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(2);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // Now delete again:
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    for (i = 0; i < r.length; i++) {
      c.mvccRemove(r[i]._key);
    }
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(2);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    for (i = 0; i < r.length; i++) {
      c.mvccRemove(r[i]._key);
    }
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    r = c.mvccAllQuery();
    for (i = 0; i < r.length; i++) {
      c.mvccRemove(r[i]._key);
    }
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);
  });

});

////////////////////////////////////////////////////////////////////////////////
/// @brief test MVCC unique skiplist index non-sparse
////////////////////////////////////////////////////////////////////////////////

describe("MVCC unique skiplist index non-sparse", function () {
  'use strict';

  var cn = "UnitTestsMvccHashIndex";

  beforeEach(function () {
    db._useDatabase("_system");
    db._drop(cn);
    var c = db._create(cn);
    c.ensureIndex({type:"skiplist", fields: ["value"], sparse: false, unique: true});
  });

  afterEach(function () {
    db._drop(cn);
  });

////////////////////////////////////////////////////////////////////////////////
/// @brief test non-sparse non-unique skiplist index and low level query function
////////////////////////////////////////////////////////////////////////////////
  
  it("should do a non-sparse unique skiplist index correctly", function () {
    var c = db[cn];
    var idx = c.getIndexes()[1];

    // Empty:
    var r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r).toEqual([]);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r).toEqual([]);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r).toEqual([]);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r).toEqual([]);

    // One document:
    c.mvccInsert({value : 1});
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // Two documents unique constraint violated:
    var error;
    try {
      c.mvccInsert({value : 1});
      error = {};
    }
    catch (e) {
      error = e;
    }
    expect(error.errorNum).toEqual(1210);
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // Three documents, different values:
    c.mvccInsert({value : 2});
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // A thousand documents:
    var i;
    for (i = 3; i < 1001; i++) {
      c.mvccInsert({value : i});
    }
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : 147 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // A document with value: null
    c.mvccInsert({value : null});
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(1);

    // Another document with value: null
    try {
      c.mvccInsert({value : null});
      error = {};
    }
    catch (e) {
      error = e;
    }
    expect(error.errorNum).toEqual(1210);

    // A document without a bound value
    try {
      c.mvccInsert({});
      error = {};
    }
    catch (e) {
      error = e;
    }
    expect(error.errorNum).toEqual(1210);

    // Remove the document again:
    r = c.mvccByExampleSkiplist(idx, { value : null });
    c.mvccRemove(r[0]._key);

    // A document with unbound value:
    c.mvccInsert({});
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(1);

    // Another document with value: null
    try {
      c.mvccInsert({value : null});
      error = {};
    }
    catch (e) {
      error = e;
    }
    expect(error.errorNum).toEqual(1210);

    // A document without a bound value
    try {
      c.mvccInsert({});
      error = {};
    }
    catch (e) {
      error = e;
    }
    expect(error.errorNum).toEqual(1210);

    // Now delete again:
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    for (i = 0; i < r.length; i++) {
      c.mvccRemove(r[i]._key);
    }
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(1);

    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    for (i = 0; i < r.length; i++) {
      c.mvccRemove(r[i]._key);
    }
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(1);

    r = c.mvccAllQuery();
    for (i = 0; i < r.length; i++) {
      c.mvccRemove(r[i]._key);
    }
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);
  });
      
});

////////////////////////////////////////////////////////////////////////////////
/// @brief test MVCC unique skiplist index sparse
////////////////////////////////////////////////////////////////////////////////

describe("MVCC unique skiplist index sparse", function () {
  'use strict';

  var cn = "UnitTestsMvccHashIndex";

  beforeEach(function () {
    db._useDatabase("_system");
    db._drop(cn);
    var c = db._create(cn);
    c.ensureIndex({type:"skiplist", fields: ["value"], sparse: true, unique: true});
  });

  afterEach(function () {
    db._drop(cn);
  });

////////////////////////////////////////////////////////////////////////////////
/// @brief test sparse non-unique skiplist index and low level query function
////////////////////////////////////////////////////////////////////////////////
  
  it("should do a sparse unique skiplist index correctly", function () {
    var c = db[cn];
    var idx = c.getIndexes()[1];

    // Empty:
    var r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r).toEqual([]);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r).toEqual([]);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r).toEqual([]);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r).toEqual([]);

    // One document:
    c.mvccInsert({value : 1});
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // Two documents unique constraint violated:
    var error;
    try {
      c.mvccInsert({value : 1});
      error = {};
    }
    catch (e) {
      error = e;
    }
    expect(error.errorNum).toEqual(1210);
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // Three documents, different values:
    c.mvccInsert({value : 2});
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // A thousand documents:
    var i;
    for (i = 3; i < 1001; i++) {
      c.mvccInsert({value : i});
    }
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : 147 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // A document with value: null
    c.mvccInsert({value : null});
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // Another document with value: null
    try {
      c.mvccInsert({value : null});
      error = {};
    }
    catch (e) {
      error = e;
    }
    expect(error).toEqual({});
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // A document without a bound value
    try {
      c.mvccInsert({});
      error = {};
    }
    catch (e) {
      error = e;
    }
    expect(error).toEqual({});
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // A document with unbound value:
    c.mvccInsert({});
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // Another document with value: null
    try {
      c.mvccInsert({value : null});
      error = {};
    }
    catch (e) {
      error = e;
    }
    expect(error).toEqual({});
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // A document without a bound value
    try {
      c.mvccInsert({});
      error = {};
    }
    catch (e) {
      error = e;
    }
    expect(error).toEqual({});
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(1);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    // Now delete again:
    var j;
    for (j = 1; j <= 1000; j++) {
      r = c.mvccByExampleSkiplist(idx, { value : j });
      for (i = 0; i < r.length; i++) {
        c.mvccRemove(r[i]._key);
      }
    }
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);

    r = c.mvccAllQuery();
    expect(r.length).toEqual(6);
    for (i = 0; i < r.length; i++) {
      c.mvccRemove(r[i]._key);
    }
    r = c.mvccByExampleSkiplist(idx, { value : 1 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : 2 });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { value : null });
    expect(r.length).toEqual(0);
    r = c.mvccByExampleSkiplist(idx, { });
    expect(r.length).toEqual(0);
  });
      
});

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// @addtogroup\\|// --SECTION--\\|/// @page\\|/// @}\\)"
// End:

