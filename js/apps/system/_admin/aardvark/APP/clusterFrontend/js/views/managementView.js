/*jshint browser: true */
/*jshint unused: false */
/*global Backbone, templateEngine, $, window, arangoHelper*/
(function () {
  "use strict";
  window.ManagementView = Backbone.View.extend({
    el: '#content',

    events: {
    },

    initialize: function () {
    },

    template: templateEngine.createTemplate("managementView.ejs"),

    render: function () {
      $(this.el).html(this.template.render());
    }

  });
}());
