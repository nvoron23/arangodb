/*jshint strict: false */
/*global require, describe, expect, beforeEach, afterEach, it */
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

var internal = require("internal");
var db = internal.db;

// -----------------------------------------------------------------------------
// --SECTION--                                                  database methods
// -----------------------------------------------------------------------------

////////////////////////////////////////////////////////////////////////////////
/// @brief test suite: dropping databases while holding references
////////////////////////////////////////////////////////////////////////////////

describe("MVCC hash index non-sparse", function () {
  'use strict';

  var cn = "UnitTestsMvccHashIndex";
  var cn2 = "UnitTestsMvccUniqueConstraint";

  beforeEach(function () {
    db._useDatabase("_system");
    db._drop(cn);
    var c = db._create(cn);
    c.ensureHashIndex("value");
    db._drop(cn2);
    var c2 = db._create(cn2);
    c2.ensureUniqueConstraint("value");
  });

  afterEach(function () {
    db._drop(cn);
    db._drop(cn2);
  });

////////////////////////////////////////////////////////////////////////////////
/// @brief test non-sparse non-unique hash index and low level query function
////////////////////////////////////////////////////////////////////////////////
  
  it("should do a non-sparse non-unique hash index correctly", function () {
    var c = db[cn];
    var idx = c.getIndexes()[1];

    // Empty:
    var r = c.mvccByExampleHash(idx, { value : 1 });
    expect(r.documents).toEqual([]);
    r = c.mvccByExampleHash(idx, { value : 2 });
    expect(r.documents).toEqual([]);
    r = c.mvccByExampleHash(idx, { value : null });
    expect(r.documents).toEqual([]);
    r = c.mvccByExampleHash(idx, { });
    expect(r.documents).toEqual([]);

    // One document:
    c.mvccInsert({value : 1});
    r = c.mvccByExampleHash(idx, { value : 1 });
    expect(r.count).toEqual(1);
    r = c.mvccByExampleHash(idx, { value : 2 });
    expect(r.count).toEqual(0);
    r = c.mvccByExampleHash(idx, { value : null });
    expect(r.count).toEqual(0);
    r = c.mvccByExampleHash(idx, { });
    expect(r.count).toEqual(0);

    // Two documents:
    c.mvccInsert({value : 1});
    r = c.mvccByExampleHash(idx, { value : 1 });
    expect(r.count).toEqual(2);
    r = c.mvccByExampleHash(idx, { value : 2 });
    expect(r.count).toEqual(0);
    r = c.mvccByExampleHash(idx, { value : null });
    expect(r.count).toEqual(0);
    r = c.mvccByExampleHash(idx, { });
    expect(r.count).toEqual(0);

    // Three documents, different values:
    c.mvccInsert({value : 2});
    r = c.mvccByExampleHash(idx, { value : 1 });
    expect(r.count).toEqual(2);
    r = c.mvccByExampleHash(idx, { value : 2 });
    expect(r.count).toEqual(1);
    r = c.mvccByExampleHash(idx, { value : null });
    expect(r.count).toEqual(0);
    r = c.mvccByExampleHash(idx, { });
    expect(r.count).toEqual(0);

    // A thousand documents:
    var i;
    for (i = 0; i < 1000; i++) {
      c.mvccInsert({value : 1});
    }
    r = c.mvccByExampleHash(idx, { value : 1 });
    expect(r.count).toEqual(1002);
    r = c.mvccByExampleHash(idx, { value : 2 });
    expect(r.count).toEqual(1);
    r = c.mvccByExampleHash(idx, { value : null });
    expect(r.count).toEqual(0);
    r = c.mvccByExampleHash(idx, { });
    expect(r.count).toEqual(0);

    // A thousand more documents:
    for (i = 0; i < 1000; i++) {
      c.mvccInsert({value : i});
    }
    r = c.mvccByExampleHash(idx, { value : 1 });
    expect(r.count).toEqual(1003);
    r = c.mvccByExampleHash(idx, { value : 2 });
    expect(r.count).toEqual(2);
    r = c.mvccByExampleHash(idx, { value : null });
    expect(r.count).toEqual(0);
    r = c.mvccByExampleHash(idx, { });
    expect(r.count).toEqual(0);

    // Some documents with value: null
    c.mvccInsert({value : null});
    c.mvccInsert({value : null});
    c.mvccInsert({value : null});
    r = c.mvccByExampleHash(idx, { value : 1 });
    expect(r.count).toEqual(1003);
    r = c.mvccByExampleHash(idx, { value : 2 });
    expect(r.count).toEqual(2);
    r = c.mvccByExampleHash(idx, { value : null });
    expect(r.count).toEqual(3);
    r = c.mvccByExampleHash(idx, { });
    expect(r.count).toEqual(3);

    // Some documents without a bound value
    c.mvccInsert({});
    c.mvccInsert({});
    c.mvccInsert({});
    r = c.mvccByExampleHash(idx, { value : 1 });
    expect(r.count).toEqual(1003);
    r = c.mvccByExampleHash(idx, { value : 2 });
    expect(r.count).toEqual(2);
    r = c.mvccByExampleHash(idx, { value : null });
    expect(r.count).toEqual(6);
    r = c.mvccByExampleHash(idx, { });
    expect(r.count).toEqual(6);
  });
      
});

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// @addtogroup\\|// --SECTION--\\|/// @page\\|/// @}\\)"
// End:

