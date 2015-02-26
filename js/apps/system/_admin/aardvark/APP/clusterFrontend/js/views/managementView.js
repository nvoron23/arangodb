/*jshint browser: true */
/*jshint unused: false */
/*global Backbone, templateEngine, $, window, arangoHelper*/
(function () {
  "use strict";
  window.ManagementView = Backbone.View.extend({
    el: '#content',

    dummyValues: [10,8,5,7,4,4,1],

    events: {
    },

    initialize: function () {
    },

    template: templateEngine.createTemplate("managementView.ejs"),

    render: function () {
      $(this.el).html(this.template.render());
      this.testFunction();
      this.renderSparklines();
    },

    renderTableLine: function () {
      var htmlString = '<div class="t-row"><div class="pure-u-3-24"><i class="fa fa-check-circle-o state-green"></i></div>';
      htmlString += '<div class="pure-u-7-24">Node A</div>';
      htmlString += '<div class="pure-u-7-24"><span class="inlinesparkline">Loading...</span>77%</div>';
      htmlString += '<div class="pure-u-7-24"><span class="inlinesparkline">Loading...</span>65%</div></div>';
      $('.management-tbody').append(htmlString);
    },

    renderSparklines: function () {
      $('.inlinesparkline').sparkline(this.dummyValues, {
        fillColor: '#8aa249'
      });
    },

    testFunction: function () {
      var i;

      for (i=0; i<10; i++) {
        this.renderTableLine();
      }
    }

  });
}());
