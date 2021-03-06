/*jslint indent: 2, nomen: true, maxlen: 100, white: true, plusplus: true, unparam: true */
/*global applicationContext */

////////////////////////////////////////////////////////////////////////////////
/// @brief An example Foxx-Application for ArangoDB
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2010-2013 triagens GmbH, Cologne, Germany
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
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

(function() {
  'use strict';

  // initialize a new FoxxApplication
  var FoxxApplication = require("org/arangodb/foxx").Controller;
  var controller = new FoxxApplication(applicationContext);

  // include console module so we can log something (in the server's log)
  var console = require("console");
  var ArangoError = require("org/arangodb").ArangoError;

  // we also need this module for custom responses
  var actions = require("org/arangodb/actions");

  // use joi for validation 
  var joi = require("joi");

  // our app is about the following Aztec deities:
  var deities = [
    "CentzonTotochtin",
    "Chalchihuitlicue",
    "Chantico",
    "Chicomecoatl",
    "Cihuacoatl",
    "Cinteotl",
    "Coatlicue",
    "Coyolxauhqui",
    "Huehuecoyotl",
    "Huitzilopochtli",
    "Ilamatecuhtli",
    "Itzcoliuhqui",
    "Itzpaplotl",
    "Mayauel",
    "Meztli",
    "Mictlantecuhtli",
    "Mixcoatl",
    "Quetzalcoatl",
    "Tezcatlipoca",
    "Tialoc",
    "Tlauixcalpantecuhtli",
    "Tlazolteotl",
    "Tonatiuh",
    "Tozi",
    "XipeTotec",
    "Xochiquetzal",
    "Xolotl",
    "Yacatecuhtli"
  ];

  // install index route (this is the default route mentioned in manifest.json)
  // this route will create an HTML overview page
  controller.get('/index', function (req, res) {
    res.contentType = "text/html";

    var body = "<h1>" + applicationContext.name + " (" + applicationContext.version + ")</h1>";
    body += "<h2>an example application demoing a few Foxx features</h2>";

    deities.forEach(function(deity) {
      body += "summon <a href=\"" + encodeURIComponent(deity) + "/summon\">" + deity + "</a><br />";
    });

    body += "<hr />pick a <a href=\"random\">random</a> Aztec deity";

    res.body = body;
  })
  .summary("prints an overview page");

  // install route to return a random deity name in JSON
  controller.get('/random', function (req, res) {
    var idx = Math.round(Math.random() * (deities.length - 1));
    res.json({ name: deities[idx] });
  })
  .summary("returns a random deity name");
  
  // install deity-specific route for summoning
  // deity name is passed as part of the URL
  controller.get('/:deity/summon', function (req, res) {
    var deity = req.params("deity");

    console.log("request to summon %s", deity);

    if (deities.indexOf(deity) === -1) {
      // unknown deity
      throw new ArangoError();
    }

    console.log("summoning %s", deity);
    
    res.json({ name: deity, summoned: true });
  })
  .summary("summons the requested deity")
  .pathParam("deity", {
    type: joi.string().required()
  })
  .errorResponse(
    Error, actions.HTTP_NOT_FOUND, "The requested deity could not be found", function(e) {
      return {
        error: true,
        code: actions.HTTP_NOT_FOUND,
        errorMessage: "The requested deity could not be found"
      };
    }
  );

}());

