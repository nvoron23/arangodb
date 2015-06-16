/*jshint strict:false, globalstrict: true, esnext: true */
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
  expect(result[0].hasOwnProperty("path")).toBeTruthy();
  expect(result[0].hasOwnProperty("vertex")).toBeTruthy();
  expect(result[0].path.hasOwnProperty("edges")).toBeTruthy();
  expect(result[0].path.hasOwnProperty("vertices")).toBeTruthy();
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

  describe("with a named graph", function () {

    let g;
    const gn = "UnitTestGraph;";

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

    describe("no steps definition", function() {

      it("should default to exactly one outbound step", function () {
        let query = "FOR x IN GRAPH @graph FROM @startId RETURN x";
        let bindVars = {
          graph: gn,
          startId: vertex.B
        };
        let result = db._query(query, bindVars);
        expect(result.length).toEqual(1);
        let entry = result[0];
        isWellFormedResult(entry);
        expect(entry.vertex._id).toEqual(vertex.C);
        expect(entry.path.vertices.length).toEqual(2);
        expect(entry.path.vertices[0]).toEqual(vertex.B);
        expect(entry.path.vertices[1]).toEqual(vertex.C);
        expect(entry.path.edges.length).toEqual(1);
        expect(entry.path.edges[0]).toEqual(edge.BC);
      });

      it("should be able to explicitly use outbound direction, equivalent to none", function () {
        let query = "FOR x IN GRAPH @graph FROM @startId OUTBOUND RETURN x";
        let bindVars = {
          graph: gn,
          startId: vertex.B
        };
        let result = db._query(query, bindVars);
        expect(result.length).toEqual(1);
        let entry = result[0];
        isWellFormedResult(entry);
        expect(entry.vertex._id).toEqual(vertex.C);
        expect(entry.path.vertices.length).toEqual(2);
        expect(entry.path.vertices[0]).toEqual(vertex.B);
        expect(entry.path.vertices[1]).toEqual(vertex.C);
        expect(entry.path.edges.length).toEqual(1);
        expect(entry.path.edges[0]).toEqual(edge.BC);
      });

      it("should be able to explicitly use inbound direction", function () {
        let query = "FOR x IN GRAPH @graph FROM @startId INBOUND RETURN x";
        let bindVars = {
          graph: gn,
          startId: vertex.C
        };
        let result = db._query(query, bindVars);
        expect(result.length).toEqual(1);
        let entry = result[0];
        isWellFormedResult(entry);
        expect(entry.vertex._id).toEqual(vertex.B);
        expect(entry.path.vertices.length).toEqual(2);
        expect(entry.path.vertices[0]).toEqual(vertex.C);
        expect(entry.path.vertices[1]).toEqual(vertex.B);
        expect(entry.path.edges.length).toEqual(1);
        expect(entry.path.edges[0]).toEqual(edge.BC);
      });

      it("should be able to sort the result", function () {
        let query = "FOR x IN GRAPH @graph FROM @startId SORT x.vertex._id ASC RETURN x";
        let bindVars = {
          graph: gn,
          startId: vertex.C
        };
        let isCD = function(entry) {
          expect(entry.vertex._id).toEqual(vertex.D);
          expect(entry.path.vertices.length).toEqual(2);
          expect(entry.path.vertices[0]).toEqual(vertex.C);
          expect(entry.path.vertices[1]).toEqual(vertex.D);
          expect(entry.path.edges.length).toEqual(1);
          expect(entry.path.edges[0]).toEqual(edge.CD);
        };

        let isCF = function(entry) {
          expect(entry.vertex._id).toEqual(vertex.F);
          expect(entry.path.vertices.length).toEqual(2);
          expect(entry.path.vertices[0]).toEqual(vertex.C);
          expect(entry.path.vertices[1]).toEqual(vertex.F);
          expect(entry.path.edges.length).toEqual(1);
          expect(entry.path.edges[0]).toEqual(edge.CF);
        };

        let result = db._query(query, bindVars);
        expect(result.length).toEqual(2);

        let entry = result[0];
        isWellFormedResult(result[0]);
        isCD(result[0]);
        isWellFormedResult(result[1]);
        isCF(result[1]);

        // Reverse ordering
        query = "FOR x IN GRAPH @graph FROM @startId SORT x.vertex._id DESC RETURN x";

        result = db._query(query, bindVars);
        expect(result.length).toEqual(2);
        entry = result[0];
        isWellFormedResult(result[0]);
        isCF(result[0]);
        isWellFormedResult(result[1]);
        isCD(result[1]);
      });

      it("should be able to explicity use any direction", function () {
        let query = "FOR x IN GRAPH @graph FROM @startId ANY SORT x.vertex._id ASC RETURN x";
        let bindVars = {
          graph: gn,
          startId: vertex.B
        };
        let result = db._query(query, bindVars);
        expect(result.length).toEqual(3);

        let entry = result[0];
        isWellFormedResult(entry);
        expect(entry.vertex._id).toEqual(vertex.A);
        expect(entry.path.vertices.length).toEqual(2);
        expect(entry.path.vertices[0]).toEqual(vertex.B);
        expect(entry.path.vertices[1]).toEqual(vertex.A);
        expect(entry.path.edges.length).toEqual(1);
        expect(entry.path.edges[0]).toEqual(edge.AB);
 
        entry = result[1];
        isWellFormedResult(entry);
        expect(entry.vertex._id).toEqual(vertex.C);
        expect(entry.path.vertices.length).toEqual(2);
        expect(entry.path.vertices[0]).toEqual(vertex.B);
        expect(entry.path.vertices[1]).toEqual(vertex.C);
        expect(entry.path.edges.length).toEqual(1);
        expect(entry.path.edges[0]).toEqual(edge.BC);

        entry = result[2];
        isWellFormedResult(entry);
        expect(entry.vertex._id).toEqual(vertex.F);
        expect(entry.path.vertices.length).toEqual(2);
        expect(entry.path.vertices[0]).toEqual(vertex.B);
        expect(entry.path.vertices[1]).toEqual(vertex.F);
        expect(entry.path.edges.length).toEqual(1);
        expect(entry.path.edges[0]).toEqual(edge.FB);
      });

    });

  });

});
