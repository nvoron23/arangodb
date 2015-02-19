/*jshint browser: true */
/*jshint unused: false */
/*global Backbone, templateEngine, $, window, arangoHelper*/
(function () {
  "use strict";
  window.HealthView = Backbone.View.extend({
    el: '#content',

    events: {
    },

    initialize: function () {
    },

    template: templateEngine.createTemplate("healthView.ejs"),

    render: function () {
      $(this.el).html(this.template.render());
    }

  });
}());
