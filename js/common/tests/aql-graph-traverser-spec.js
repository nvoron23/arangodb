/*jshint esnext: true */
/*global describe, beforeEach, it, expect, afterEach*/

////////////////////////////////////////////////////////////////////////////////
/// @brief Spec for the AQL FOR x IN GRAPH name statement
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
/// @author Michael Hackstein
/// @author Copyright 2015, ArangoDB GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

(function() {
  "use strict";

  const internal = require("internal");
  const db = internal.db;
  const arangodb = require("org/arangodb");
  const errors = arangodb.errors;
  const gm = require("org/arangodb/general-graph");
  const vn = "UnitTestVertexCollection";
  const en = "UnitTestEdgeCollection";
  const vertex = {};
  const edge = {};

  let cleanup = function () {
    db._drop(vn);
    db._drop(en);
  };

  let isWellFormedResult = function (result) {
    expect(result.hasOwnProperty("path")).toBeTruthy();
    expect(result.hasOwnProperty("vertex")).toBeTruthy();
    expect(result.path.hasOwnProperty("edges")).toBeTruthy();
    expect(result.path.hasOwnProperty("vertices")).toBeTruthy();
  };

  describe("The FOR x IN GRAPH statement", function () {

    /***********************************************************************
     * Graph under test:
     *
     *  A -> B -> C -> D
     *      /|\  \|/
     *       E <- F
     *
     *
     *
     *
     *
     *
     *
     *
     *
     *
     *
     *
     *
     * *********************************************************************/
    beforeEach(function() {
      cleanup();
      let vc = db._create(vn);
      let ec = db._createEdgeCollection(en);
      vertex.A = vc.save({_key: "A"})._id;
      vertex.B = vc.save({_key: "B"})._id;
      vertex.C = vc.save({_key: "C"})._id;
      vertex.D = vc.save({_key: "D"})._id;
      vertex.E = vc.save({_key: "E"})._id;
      vertex.F = vc.save({_key: "F"})._id;

      edge.AB = ec.save(vertex.A, vertex.B, {})._id;
      edge.BC = ec.save(vertex.B, vertex.C, {})._id;
      edge.CD = ec.save(vertex.C, vertex.D, {})._id;
      edge.CF = ec.save(vertex.C, vertex.F, {})._id;
      edge.EB = ec.save(vertex.E, vertex.B, {})._id;
      edge.FE = ec.save(vertex.F, vertex.E, {})._id;
    });

    afterEach(function() {
      cleanup();
    });

    /*
    describe("with a named graph", function () {

      let g;
      const gn = "UnitTestGraph";

      beforeEach(function() {
        try {
          gm._drop(gn);
        } catch (e) {
          // It is expected that this graph does not exist.
        }
        g = gm._create(gn, [gm._relation(en, vn, vn)]);
      });

      afterEach(function() {
        gm._drop(gn);
      });

      describe("return format", function() {

        it("should return path and vertex", function () {
          let query = "FOR x IN TRAVERSE FROM @startId GRAPH @graph 1 STEPS RETURN x";
          let bindVars = {
            graph: gn,
            startId: vertex.B
          };
          let result = db._query(query, bindVars).toArray();
          expect(result.length).toEqual(1);
          let entry = result[0];
          isWellFormedResult(entry);
          expect(entry.vertex._id).toEqual(vertex.C);
          expect(entry.path.vertices.length).toEqual(2);
          expect(entry.path.vertices[0]._id).toEqual(vertex.B);
          expect(entry.path.vertices[1]._id).toEqual(vertex.C);
          expect(entry.path.edges.length).toEqual(1);
          expect(entry.path.edges[0]._id).toEqual(edge.BC);
        });
      });

      describe("direction", function() {

        it("can use outbound direction, equivalent to default", function () {
          let query = "FOR x IN TRAVERSE OUTBOUND FROM @startId GRAPH @graph 1 STEPS RETURN x._vertex._id";
          let bindVars = {
            graph: gn,
            startId: vertex.B
          };
          let result = db._query(query, bindVars).toArray();
          expect(result.length).toEqual(1);
          let entry = result[0];
          expect(entry).toEqual(vertex.C);
        });

        it("can use inbound direction", function () {
          let query = "FOR x IN TRAVERSE INBOUND FROM @startId GRAPH @graph 1 STEPS RETURN x._vertex._id";
          let bindVars = {
            graph: gn,
            startId: vertex.C
          };
          let result = db._query(query, bindVars).toArray();
          expect(result.length).toEqual(1);
          let entry = result[0];
          expect(entry).toEqual(vertex.B);
        });

        it("can use any direction", function () {
          let query = "FOR x IN TRAVERSE ANY FROM @startId GRAPH @graph 1 STEPS "
                    + "SORT x._vertex._id ASC RETURN x._vertex._id";
          let bindVars = {
            graph: gn,
            startId: vertex.B
          };
          let result = db._query(query, bindVars).toArray();
          expect(result.length).toEqual(3);

          let entry = result[0];
          expect(entry).toEqual(vertex.A);
          entry = result[1];
          expect(entry).toEqual(vertex.C);
          entry = result[2];
          expect(entry).toEqual(vertex.F);
        });

      });

      describe("steps", function () {

        it("can use an exact number of steps", function () {
          let query = "FOR x IN TRAVERSE FROM @startId GRAPH @graph 2 STEPS "
                    + "SORT x._vertex._id ASC RETURN x._vertex._id";
          let bindVars = {
            graph: gn,
            startId: vertex.B
          };
          let result = db._query(query, bindVars).toArray();
          expect(result.length).toEqual(2);

          expect(result[0]).toEqual(vertex.D);
          expect(result[1]).toEqual(vertex.F);
        });

        it("can use a range of steps", function () {
          let query = "FOR x IN TRAVERSE FROM @startId GRAPH @graph 2..3 STEPS "
                    + "SORT x._vertex._id ASC RETURN x._vertex._id";
          let bindVars = {
            graph: gn,
            startId: vertex.B
          };
          let result = db._query(query, bindVars).toArray();
          expect(result.length).toEqual(3);

          expect(result[0]).toEqual(vertex.D);
          expect(result[1]).toEqual(vertex.E);
          expect(result[2]).toEqual(vertex.F);
        });

        it("can use a computed function of steps", function () {
          let query = "FOR x IN TRAVERSE FROM @startId GRAPH @graph LENGTH([1, 2]) STEPS "
                    + "SORT x._vertex._id ASC RETURN x._vertex._id";
          let bindVars = {
            graph: gn,
            startId: vertex.B
          };
          let result = db._query(query, bindVars).toArray();
          expect(result.length).toEqual(3);

          expect(result[0]).toEqual(vertex.D);
          expect(result[1]).toEqual(vertex.F);
        });

      });

      describe("sorting", function () {
        it("should be able to sort the result", function () {
          let query = "FOR x IN TRAVERSE FROM @startId GRAPH @graph 1 STEPS " 
                    + "SORT x._vertex._id ASC RETURN x._vertex._id";
          let bindVars = {
            graph: gn,
            startId: vertex.C
          };

          let result = db._query(query, bindVars).toArray();
          expect(result.length).toEqual(2);
          expect(result[0]).toEqual(vertex.D);
          expect(result[1]).toEqual(vertex.F);

          // Reverse ordering
          query = "FOR x IN TRAVERSE FROM @startId GRAPH @graph 1 STEPS SORT x._vertex._id DESC RETURN x._vertex._id";

          result = db._query(query, bindVars).toArray();
          expect(result.length).toEqual(2);
          expect(result[0]).toEqual(vertex.F);
          expect(result[1]).toEqual(vertex.D);
        });

      });

    });
    */

    describe("with a collection pair", function () {

      describe("bind parameter positions", function () {

        // We always use the same query, the result should be identical.
        let validateResult = function (result) {
          expect(result.length).toEqual(1);
          let entry = result[0];
          isWellFormedResult(entry);
          expect(entry.vertex._id).toEqual(vertex.C);
          expect(entry.path.vertices.length).toEqual(2);
          expect(entry.path.vertices[0]._id).toEqual(vertex.B);
          expect(entry.path.vertices[1]._id).toEqual(vertex.C);
          expect(entry.path.edges.length).toEqual(1);
          expect(entry.path.edges[0]._id).toEqual(edge.BC);
        };

        it("should be able to use no bind parameters", function () {
          let query = "FOR x IN TRAVERSE FROM '" + vertex.B + "' GRAPH " + en + ", " + vn + " 1 STEPS RETURN x";
          validateResult(db._query(query).toArray());
        });

        it("should be able to bind the start point", function () {
          let query = "FOR x IN TRAVERSE FROM @startId GRAPH " + en + ", " + vn + " 1 STEPS RETURN x";
          let bindVars = {
            startId: vertex.B
          };
          validateResult(db._query(query, bindVars).toArray());
        });

        it("should be able to bind the edge collection", function () {
          let query = "FOR x IN TRAVERSE FROM '" + vertex.B + "' GRAPH @@eCol, " + vn + " 1 STEPS RETURN x";
          let bindVars = {
            "@eCol": en
          };
          validateResult(db._query(query, bindVars).toArray());
        });

        it("should be able to bind the vertex collection", function () {
          let query = "FOR x IN TRAVERSE FROM '" + vertex.B + "' GRAPH " + en + ", @@vCol 1 STEPS RETURN x";
          let bindVars = {
            "@vCol": vn
          };
          validateResult(db._query(query, bindVars).toArray());
        });

        it("should be able to bind both collections", function () {
          let query = "FOR x IN TRAVERSE FROM '" + vertex.B + "' GRAPH @@eCol, @@vCol 1 STEPS RETURN x";
          let bindVars = {
            "@eCol": en,
            "@vCol": vn
          };
          validateResult(db._query(query, bindVars).toArray());
        });

        it("should be able to bind the steps", function () {
          let query = "FOR x IN TRAVERSE FROM '" + vertex.B + "' GRAPH " + en + ", " + vn + " @steps STEPS RETURN x";
          let bindVars = {
            "steps": 1
          };
          validateResult(db._query(query, bindVars).toArray());
        });

        it("should be able to bind the steps as range with two values", function () {
          let query = "FOR x IN TRAVERSE FROM '" + vertex.B + "' GRAPH " + en + ", "
                    + vn + " @lsteps..@rsteps STEPS RETURN x";
          let bindVars = {
            "lsteps": 1,
            "rsteps": 1
          };
          validateResult(db._query(query, bindVars).toArray());
        });

        /* TODO: Should we support this?
        it("should be able to bind the steps as range in one value", function () {
          let query = "FOR x IN TRAVERSE FROM '" + vertex.B + "' GRAPH " + en + ", "
                    + vn + " @steps STEPS RETURN x";
          let bindVars = {
            "steps": "1..1",
          };
          validateResult(db._query(query, bindVars).toArray());
        });
        */

      });

      describe("return format", function() {

        it("should return path and vertex", function () {
          let query = "FOR x IN TRAVERSE FROM @startId GRAPH @@eCol, @@vCol 1 STEPS RETURN x";
          let bindVars = {
            "@eCol": en,
            "@vCol": vn,
            startId: vertex.B
          };
          let result = db._query(query, bindVars).toArray();
          expect(result.length).toEqual(1);
          let entry = result[0];
          isWellFormedResult(entry);
          expect(entry.vertex._id).toEqual(vertex.C);
          expect(entry.path.vertices.length).toEqual(2);
          expect(entry.path.vertices[0]._id).toEqual(vertex.B);
          expect(entry.path.vertices[1]._id).toEqual(vertex.C);
          expect(entry.path.edges.length).toEqual(1);
          expect(entry.path.edges[0]._id).toEqual(edge.BC);
        });
      });

      describe("direction", function() {

        it("can use outbound direction, equivalent to default", function () {
          let query = "FOR x IN TRAVERSE 'outbound' FROM @startId GRAPH @@eCol, @@vCol 1 STEPS RETURN x.vertex._id";
          let bindVars = {
            "@eCol": en,
            "@vCol": vn,
            startId: vertex.B
          };
          let result = db._query(query, bindVars).toArray();
          expect(result.length).toEqual(1);
          let entry = result[0];
          expect(entry).toEqual(vertex.C);
        });

        it("can use inbound direction", function () {
          let query = "FOR x IN TRAVERSE 'inbound' FROM @startId GRAPH @@eCol, @@vCol 1 STEPS RETURN x.vertex._id";
          let bindVars = {
            "@eCol": en,
            "@vCol": vn,
            startId: vertex.C
          };
          let result = db._query(query, bindVars).toArray();
          expect(result.length).toEqual(1);
          let entry = result[0];
          expect(entry).toEqual(vertex.B);
        });

        it("can use any direction", function () {
          let query = "FOR x IN TRAVERSE 'any' FROM @startId GRAPH @@eCol, @@vCol 1 STEPS "
                    + "SORT x.vertex._id ASC RETURN x.vertex._id";
          let bindVars = {
            "@eCol": en,
            "@vCol": vn,
            startId: vertex.B
          };
          let result = db._query(query, bindVars).toArray();
          expect(result.length).toEqual(3);

          let entry = result[0];
          expect(entry).toEqual(vertex.A);
          entry = result[1];
          expect(entry).toEqual(vertex.C);
          entry = result[2];
          expect(entry).toEqual(vertex.E);
        });

      });

      describe("steps", function () {

        it("can use an exact number of steps", function () {
          let query = "FOR x IN TRAVERSE FROM @startId GRAPH @@eCol, @@vCol 2 STEPS "
                    + "SORT x.vertex._id ASC RETURN x.vertex._id";
          let bindVars = {
            "@eCol": en,
            "@vCol": vn,
            startId: vertex.B
          };
          let result = db._query(query, bindVars).toArray();
          expect(result.length).toEqual(2);

          expect(result[0]).toEqual(vertex.D);
          expect(result[1]).toEqual(vertex.F);
        });

        it("can use a range of steps", function () {
          let query = "FOR x IN TRAVERSE FROM @startId GRAPH @@eCol, @@vCol 2..3 STEPS "
                    + "SORT x.vertex._id ASC RETURN x.vertex._id";
          let bindVars = {
            "@eCol": en,
            "@vCol": vn,
            startId: vertex.B
          };
          let result = db._query(query, bindVars).toArray();
          expect(result.length).toEqual(3);

          expect(result[0]).toEqual(vertex.D);
          expect(result[1]).toEqual(vertex.E);
          expect(result[2]).toEqual(vertex.F);
        });

        it("can use a computed function of steps", function () {
          let query = "FOR x IN TRAVERSE FROM @startId GRAPH @@eCol, @@vCol LENGTH([1, 2]) STEPS "
                    + "SORT x.vertex._id ASC RETURN x.vertex._id";
          let bindVars = {
            "@eCol": en,
            "@vCol": vn,
            startId: vertex.B
          };
          let result = db._query(query, bindVars).toArray();
          expect(result.length).toEqual(2);

          expect(result[0]).toEqual(vertex.D);
          expect(result[1]).toEqual(vertex.F);
        });

      });

      describe("sorting", function () {
        it("should be able to sort the result", function () {
          let query = "FOR x IN TRAVERSE FROM @startId GRAPH @@eCol, @@vCol 1 STEPS " 
                    + "SORT x.vertex._id ASC RETURN x.vertex._id";
          let bindVars = {
            "@eCol": en,
            "@vCol": vn,
            startId: vertex.C
          };

          let result = db._query(query, bindVars).toArray();
          expect(result.length).toEqual(2);
          expect(result[0]).toEqual(vertex.D);
          expect(result[1]).toEqual(vertex.F);

          // Reverse ordering
          query = "FOR x IN TRAVERSE FROM @startId GRAPH @@eCol, @@vCol 1 STEPS "
                + "SORT x.vertex._id DESC RETURN x.vertex._id";

          result = db._query(query, bindVars).toArray();
          expect(result.length).toEqual(2);
          expect(result[0]).toEqual(vertex.F);
          expect(result[1]).toEqual(vertex.D);
        });

      });

    });


  });

}());
