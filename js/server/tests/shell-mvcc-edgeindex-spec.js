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
/// @author Jan Steemann
/// @author Copyright 2015, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

var internal = require("internal");
var db = internal.db;

////////////////////////////////////////////////////////////////////////////////
/// @brief test MVCC hash index non-sparse
////////////////////////////////////////////////////////////////////////////////

describe("MVCC edge index", function () {
  'use strict';

  var cnAux = "UnitTestsMvccVertex";
  var cn = "UnitTestsMvccEdge";

  beforeEach(function () {
    db._useDatabase("_system");
    db._drop(cnAux);
    db._drop(cn);

    db._create(cnAux);
    db._createEdgeCollection(cn);
  });

  afterEach(function () {
    db._drop(cnAux);
    db._drop(cn);
  });

////////////////////////////////////////////////////////////////////////////////
/// @brief test edge index query function
////////////////////////////////////////////////////////////////////////////////
  
  it("should fail when calling it on a non-edge collection", function () {
    var c = db[cnAux];

    try {
      c.mvccOutEdges(cnAux + "/1");
      expect(false).toEqual(true);
    }
    catch (err) {
      expect(err.errorNum).toEqual(internal.errors.ERROR_ARANGO_COLLECTION_TYPE_INVALID.code);
    }
  });

////////////////////////////////////////////////////////////////////////////////
/// @brief test edge index query function
////////////////////////////////////////////////////////////////////////////////
  
  it("should produce no results for a non-existing edge", function () {
    var c = db[cn];

    var r = c.mvccOutEdges("this-does-not-exist/1");
    expect(r).toEqual([]);
    r = c.mvccInEdges("this-does-not-exist/1");
    expect(r).toEqual([]);
    r = c.mvccEdges("this-does-not-exist/1");
    expect(r).toEqual([]);
  });

////////////////////////////////////////////////////////////////////////////////
/// @brief test edge index query function
////////////////////////////////////////////////////////////////////////////////
  
  it("should produce no results for non-existing edges, array lookup", function () {
    var c = db[cn];

    var r = c.mvccOutEdges([ "this-does-not-exist/1", "this-does-not-exist/2" ]);
    expect(r).toEqual([]);
    r = c.mvccInEdges([ "this-does-not-exist/1", "this-does-not-exist/2" ]);
    expect(r).toEqual([]);
    r = c.mvccEdges([ "this-does-not-exist/1", "this-does-not-exist/2" ]);
    expect(r).toEqual([]);
  });

////////////////////////////////////////////////////////////////////////////////
/// @brief test edge index query function
////////////////////////////////////////////////////////////////////////////////
  
  it("should produce no results for an empty collection", function () {
    var c = db[cn];

    // Empty:
    var r = c.mvccOutEdges(cnAux + "/1");
    expect(r).toEqual([]);
    r = c.mvccInEdges(cnAux + "/1");
    expect(r).toEqual([]);
    r = c.mvccEdges(cnAux + "/1");
    expect(r).toEqual([]);
  });

////////////////////////////////////////////////////////////////////////////////
/// @brief test edge index query function
////////////////////////////////////////////////////////////////////////////////
  
  it("should produce no results for an empty collection, array lookup", function () {
    var c = db[cn];

    // Empty:
    var r = c.mvccOutEdges([ cnAux + "/1" ]);
    expect(r).toEqual([]);
    r = c.mvccInEdges([ cnAux + "/1" ]);
    expect(r).toEqual([]);
    r = c.mvccEdges([ cnAux + "/1" ]);
    expect(r).toEqual([]);
  });

////////////////////////////////////////////////////////////////////////////////
/// @brief test edge index query function
////////////////////////////////////////////////////////////////////////////////

  it("should produce a result for a single-document collection", function () {
    var c = db[cn];

    // One document:
    c.mvccInsert(cnAux + "/1", cnAux + "/2", { type: "test" });
    
    var r = c.mvccOutEdges(cnAux + "/1");
    expect(r.length).toEqual(1);
    expect(r[0]._from).toEqual(cnAux + "/1");
    expect(r[0]._to).toEqual(cnAux + "/2");
    r = c.mvccOutEdges(cnAux + "/2");
    expect(r.length).toEqual(0);

    r = c.mvccInEdges(cnAux + "/1");
    expect(r.length).toEqual(0);
    r = c.mvccInEdges(cnAux + "/2");
    expect(r.length).toEqual(1);
    expect(r[0]._from).toEqual(cnAux + "/1");
    expect(r[0]._to).toEqual(cnAux + "/2");
    
    r = c.mvccEdges(cnAux + "/1");
    expect(r.length).toEqual(1);
    expect(r[0]._from).toEqual(cnAux + "/1");
    expect(r[0]._to).toEqual(cnAux + "/2");

    r = c.mvccEdges(cnAux + "/2");
    expect(r.length).toEqual(1);
    expect(r[0]._from).toEqual(cnAux + "/1");
    expect(r[0]._to).toEqual(cnAux + "/2");
  });

////////////////////////////////////////////////////////////////////////////////
/// @brief test edge index query function
////////////////////////////////////////////////////////////////////////////////

  it("should produce a result for array lookup", function () {
    var c = db[cn];

    var i;
    for (i = 0; i < 100; ++i) {
      c.mvccInsert(cnAux + "/a" + i, cnAux + "/b" + i, { _key: "test" + i, type: "test" + i });
    }
    
    var r = c.mvccOutEdges([ cnAux + "/a1", cnAux + "/a2", cnAux + "/gnfng" ]);
    expect(r.length).toEqual(2);
    expect(r[0]._from).toEqual(cnAux + "/a1");
    expect(r[0]._to).toEqual(cnAux + "/b1");

    expect(r[1]._from).toEqual(cnAux + "/a2");
    expect(r[1]._to).toEqual(cnAux + "/b2");
    
    r = c.mvccInEdges([ cnAux + "/b1", cnAux + "/b2", cnAux + "/gnfng" ]);
    expect(r.length).toEqual(2);
    expect(r[0]._from).toEqual(cnAux + "/a1");
    expect(r[0]._to).toEqual(cnAux + "/b1");

    expect(r[1]._from).toEqual(cnAux + "/a2");
    expect(r[1]._to).toEqual(cnAux + "/b2");
    
    r = c.mvccEdges([ cnAux + "/a1", cnAux + "/a2", cnAux + "/gnfng" ]);
    expect(r.length).toEqual(2);
    expect(r[0]._from).toEqual(cnAux + "/a1");
    expect(r[0]._to).toEqual(cnAux + "/b1");

    expect(r[1]._from).toEqual(cnAux + "/a2");
    expect(r[1]._to).toEqual(cnAux + "/b2");
  });

////////////////////////////////////////////////////////////////////////////////
/// @brief test edge index query function
////////////////////////////////////////////////////////////////////////////////

  it("should produce a result for a multi-document collection", function () {
    var c = db[cn];

    var i;
    for (i = 0; i < 1000; ++i) {
      c.mvccInsert(cnAux + "/a" + (i % 100), cnAux + "/b" + (i % 100), { _key: "test" + i, type: "test" + i });
    }
    
    var r = c.mvccOutEdges(cnAux + "/a1");
    expect(r.length).toEqual(10);
    for (i = 0; i < r.length; ++i) {
      expect(r[i]._from).toEqual(cnAux + "/a1");
      expect(r[i]._to).toEqual(cnAux + "/b1");
      expect(r[i].type).toEqual(r[i].type);
    }

    r = c.mvccInEdges(cnAux + "/a1");
    expect(r.length).toEqual(0);
    r = c.mvccInEdges(cnAux + "/b1");
    expect(r.length).toEqual(10);
    for (i = 0; i < r.length; ++i) {
      expect(r[i]._from).toEqual(cnAux + "/a1");
      expect(r[i]._to).toEqual(cnAux + "/b1");
      expect(r[i].type).toEqual(r[i].type);
    }
    
    r = c.mvccEdges(cnAux + "/a1");
    expect(r.length).toEqual(10);
    for (i = 0; i < r.length; ++i) {
      expect(r[i]._from).toEqual(cnAux + "/a1");
      expect(r[i]._to).toEqual(cnAux + "/b1");
      expect(r[i].type).toEqual(r[i].type);
    }
    
    r = c.mvccInEdges(cnAux + "/b1");
    expect(r.length).toEqual(10);
    for (i = 0; i < r.length; ++i) {
      expect(r[i]._from).toEqual(cnAux + "/a1");
      expect(r[i]._to).toEqual(cnAux + "/b1");
      expect(r[i].type).toEqual(r[i].type);
    }
  });

////////////////////////////////////////////////////////////////////////////////
/// @brief test edge index query function
////////////////////////////////////////////////////////////////////////////////

  it("should return reflexive edges only once for 'any' direction", function () {
    var c = db[cn];

    c.mvccInsert(cnAux + "/a1", cnAux + "/a1", { test: 1 });
    c.mvccInsert(cnAux + "/a1", cnAux + "/a2", { test: 2 });
    c.mvccInsert(cnAux + "/a2", cnAux + "/a1", { test: 3 });
    
    var r = c.mvccOutEdges(cnAux + "/a1").sort(function(l, r) { return l.test - r .test; });
    expect(r.length).toEqual(2);
    expect(r[0]._from).toEqual(cnAux + "/a1");
    expect(r[0]._to).toEqual(cnAux + "/a1");
    expect(r[0].test).toEqual(1);

    expect(r[1]._from).toEqual(cnAux + "/a1");
    expect(r[1]._to).toEqual(cnAux + "/a2");
    expect(r[1].test).toEqual(2);
    
    r = c.mvccInEdges(cnAux + "/a1").sort(function(l, r) { return l.test - r .test; });
    expect(r.length).toEqual(2);
    expect(r[0]._from).toEqual(cnAux + "/a1");
    expect(r[0]._to).toEqual(cnAux + "/a1");
    expect(r[0].test).toEqual(1);

    expect(r[1]._from).toEqual(cnAux + "/a2");
    expect(r[1]._to).toEqual(cnAux + "/a1");
    expect(r[1].test).toEqual(3);
    
    r = c.mvccEdges(cnAux + "/a1").sort(function(l, r) { return l.test - r .test; });
    expect(r.length).toEqual(3);
    expect(r[0]._from).toEqual(cnAux + "/a1");
    expect(r[0]._to).toEqual(cnAux + "/a1");
    expect(r[0].test).toEqual(1);
    
    expect(r[1]._from).toEqual(cnAux + "/a1");
    expect(r[1]._to).toEqual(cnAux + "/a2");
    expect(r[1].test).toEqual(2);

    expect(r[2]._from).toEqual(cnAux + "/a2");
    expect(r[2]._to).toEqual(cnAux + "/a1");
    expect(r[2].test).toEqual(3);
  });
   
});

// Local Variables:
// mode: outline-minor
// outline-regexp: "^\\(/// @brief\\|/// @addtogroup\\|// --SECTION--\\|/// @page\\|/// @}\\)"
// End:

