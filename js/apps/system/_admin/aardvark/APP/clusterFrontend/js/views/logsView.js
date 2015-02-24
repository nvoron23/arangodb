/*jshint browser: true */
/*jshint unused: false */
/*global Backbone, templateEngine, $, window, arangoHelper*/
(function () {
  "use strict";
  window.LogsView = Backbone.View.extend({
    el: '#content',

    events: {
    },

    initialize: function () {
    },

    template: templateEngine.createTemplate("logsView.ejs"),

    render: function () {
      $(this.el).html(this.template.render());
      this.renderRow(
        '<p class="l-icon"><i class="fa fa-database"></i></p>',
        '<p class="l-message">No message</p><p class="l-from"> - Node1</p>',
        '<i class="fa fa-clock-o"></i>' + moment("20150224", "YYYYMMDD").fromNow()
      );
      this.renderRow(
        '<p class="l-icon"><i class="fa fa-database"></i></p>',
        '<p class="l-message">No message</p><p class="l-from"> - Node2</p>',
        '<i class="fa fa-clock-o"></i>' + moment("20150620", "YYYYMMDD").fromNow()
      );
      this.renderRow(
        '<p class="l-icon"><i class="fa fa-database"></i></p>',
        '<p class="l-message">No message</p><p class="l-from"> - Node3</p>',
        '<i class="fa fa-clock-o"></i>' + moment("20150620", "YYYYMMDD").fromNow()
      );
    },

    renderRow: function (icon, message, time) {
      var htmlString = '<div class="t-row pure-g">';
      htmlString += '<div class="pure-u-1-5"><p class="t-content">'+icon+'</p></div>';
      htmlString += '<div class="pure-u-3-5"><p class="t-content">'+message+'</p></div>';
      htmlString += '<div class="pure-u-1-5"><p class="t-content">'+time+'</p></div>';
      htmlString += '</div>';

      $('.t-cluster-body').append(htmlString);
    }

  });
}());
