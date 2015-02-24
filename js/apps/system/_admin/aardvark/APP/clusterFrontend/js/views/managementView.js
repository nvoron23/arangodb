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
      this.testFunction();
      console.log("after render");
    },

    renderTableLine: function () {
      var htmlString = '<div class="t-row"><div class="pure-u-3-24"><i class="fa fa-check-circle-o state-green"></i></div>';
      htmlString += '<div class="pure-u-7-24">Node A</div>';
      htmlString += '<div class="pure-u-7-24">77%</div>';
      htmlString += '<div class="pure-u-7-24">65%</div></div>';
      $('.management-tbody').append(htmlString);
    },

    testFunction: function() {
      var i;

      for (i=0; i<10; i++) {
        this.renderTableLine();
      }
    }

  });
}());
