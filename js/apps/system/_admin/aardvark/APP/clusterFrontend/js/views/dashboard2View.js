/*jshint browser: true */
/*jshint unused: false */
/*global Backbone, templateEngine, $, window, arangoHelper*/
(function () {
  "use strict";
  window.Dashboard2View = Backbone.View.extend({
    el: '#content',

    events: {
    },

    initialize: function () {
    },

    template: templateEngine.createTemplate("dashboard2View.ejs"),

    render: function () {
      $(this.el).html(this.template.render());
    }

  });
}());
