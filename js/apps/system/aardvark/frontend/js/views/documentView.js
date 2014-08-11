/*jslint indent: 2, nomen: true, maxlen: 100, vars: true, white: true, plusplus: true, forin: true */
/*global require, exports, Backbone, EJS, $, window, arangoHelper, jsoneditor, templateEngine */
/*global document, _ */

(function() {
  "use strict";
  window.DocumentView = Backbone.View.extend({
    el: '#content',
    colid: 0,
    docid: 0,

    template: templateEngine.createTemplate("documentView.ejs"),

    events: {
      "click #saveDocumentButton" : "saveDocument",
      "dblclick #documentEditor tr" : "addProperty"
    },

    editor: 0,

    typeCheck: function (type) {
      var result;
      if (type === 'edge') {
        result = this.collection.getEdge(this.colid, this.docid);
      }
      else if (type === 'document') {
        result = this.collection.getDocument(this.colid, this.docid);
      }
      if (result === true) {
        this.fillEditor();
        return true;
      }
    },

    fillEditor: function() {
      var toFill = this.removeReadonlyKeys(this.collection.first().attributes);
      this.editor.set(toFill);
    },

    jsonContentChanged: function() {
      this.enableSaveButton();
    },

    render: function() {
      $(this.el).html(this.template.render({}));
      this.disableSaveButton();
      this.breadcrumb();

      var self = this;

      var container = document.getElementById('documentEditor');
      var options = {
        change: function(){self.jsonContentChanged();},
        search: true,
        mode: 'tree',
        modes: ['tree', 'code']
      };
      this.editor = new window.jsoneditor.JSONEditor(container, options);

      return this;
    },

    addProperty: function (e) {
      var node, searchResult;
      try {
        node = e.currentTarget.cells[2].childNodes[0].
          childNodes[0].childNodes[0].childNodes[1].
          childNodes[0].textContent;
      } catch (ex) {

      }
      if (node) {
        if (node === "object") {
          if (_.isEmpty(this.editor.get())) {
            this.editor.set({
              "": ""
            });
            this.editor.node.childs[0].focus("field");
          } else {
            this.editor.node.childs[0]._onInsertBefore(undefined, undefined, "auto");
          }
          return;
        }
        searchResult = this.editor.node.search(node);
        var breakLoop = false;
        searchResult.forEach(function (s) {
          if (breakLoop) {
            return;
          }
          if (s.elem === "field" ) {
            s.node._onInsertAfter(undefined, undefined, "auto");
            breakLoop = true;
          }
        });

      }
    },

    removeReadonlyKeys: function (object) {
      delete object._key;
      delete object._rev;
      delete object._id;
      delete object._from;
      delete object._to;
      return object;
    },

    saveDocument: function () {
      var model, result;

      try {
        model = this.editor.get();
      }
      catch (e) {
        this.errorConfirmation();
        this.disableSaveButton();
      }

      model = JSON.stringify(model);

      if (this.type === 'document') {
        result = this.collection.saveDocument(this.colid, this.docid, model);
        if (result === false) {
          arangoHelper.arangoError('Document error:','Could not save');
          return;
        }
      }
      else if (this.type === 'edge') {
        result = this.collection.saveEdge(this.colid, this.docid, model);
        if (result === false) {
          arangoHelper.arangoError('Edge error:', 'Could not save');
          return;
        }
      }

      if (result === true) {
        this.successConfirmation();
        this.disableSaveButton();
      }
    },

    successConfirmation: function () {
      $('#documentEditor .tree').animate({backgroundColor: '#C6FFB0'}, 500);
      $('#documentEditor .tree').animate({backgroundColor: '#FFFFF'}, 500);

      $('#documentEditor .ace_content').animate({backgroundColor: '#C6FFB0'}, 500);
      $('#documentEditor .ace_content').animate({backgroundColor: '#FFFFF'}, 500);
    },

    errorConfirmation: function () {
      $('#documentEditor .tree').animate({backgroundColor: '#FFB0B0'}, 500);
      $('#documentEditor .tree').animate({backgroundColor: '#FFFFF'}, 500);

      $('#documentEditor .ace_content').animate({backgroundColor: '#FFB0B0'}, 500);
      $('#documentEditor .ace_content').animate({backgroundColor: '#FFFFF'}, 500);
    },

    enableSaveButton: function () {
      $('#saveDocumentButton').prop('disabled', false);
      $('#saveDocumentButton').addClass('button-success');
      $('#saveDocumentButton').removeClass('button-close');
    },

    disableSaveButton: function () {
      $('#saveDocumentButton').prop('disabled', true);
      $('#saveDocumentButton').addClass('button-close');
      $('#saveDocumentButton').removeClass('button-success');
    },

    breadcrumb: function () {
      var name = window.location.hash.split("/");
      $('#transparentHeader').append(
        '<div class="breadcrumb">'+
        '<a href="#collections" class="activeBread">Collections</a>'+
        '  >  '+
        '<a class="activeBread" href="#collection/' + name[1] + '/documents/1">' + name[1] + '</a>'+
        '  >  '+
        '<a class="disabledBread">' + name[2] + '</a>'+
        '</div>'
      );
    },

    escaped: function (value) {
      return value.replace(/&/g, "&amp;").replace(/</g, "&lt;").replace(/>/g, "&gt;")
      .replace(/"/g, "&quot;").replace(/'/g, "&#39;");
    }

  });
}());
