/*jshint browser: true */
/*jshint unused: false */
/*global Backbone, templateEngine, $, window, arangoHelper*/
(function () {
  "use strict";
  window.EmbeddedNavigationView = Backbone.View.extend({

    el: '.clusterNavbar',
    el2: '.clusterNavbar ul',

    dummyData: {
      state: "warning",
      nodes: "6/7",
      data: "100mb",
      cpu: 55,
      ram: 23,
      uptime: "5 days",
      version: "V1"
    },

    events: {
    },

    initialize: function () {
    },

    template: templateEngine.createTemplate("embeddedNavigationView.ejs"),

    render: function () {
      $(this.el).html(this.template.render());

      //just for dev
      this.rerender();
    },

    rerender: function() {
      var self = this;

      _.each(this.dummyData, function(val, key) {

        var string = key;
        if (typeof val === 'string') {
          string += ": " + val;
        }
        else if (typeof val === 'number') {
          if (val < 33) {
            string += ': <span class="state-green">' + val + ' %</span>';
          }
          else if (val > 33 && val < 66) {
            string += ': <span class="state-yellow">' + val + ' %</span>';
          }
          else {
            string += ': <span class="state-red">' + val + ' %</span>';
          }
        }

        jQuery('<li/>', {
          html: string
        }).appendTo($(self.el2));
      });
    }

  });
}());
