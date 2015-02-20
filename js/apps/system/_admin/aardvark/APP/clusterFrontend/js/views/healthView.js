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

    dummyJSON: {
      availability: [
        {
          name: "Pairs",
          value: "400/403",
          icon: "fa fa-cubes state-green"
        },
        {
          name: "Single",
          value: "5",
          icon: "fa fa-cube state-yellow"
        },
        {
          name: "Not Available",
          value: "4",
          icon: "fa fa-times-circle state-red"
        }
      ],
      cpu: [
        {
          name: "Low load",
          value: "1",
          icon: "fa fa-dashboard state-green"
        },
        {
          name: "Medium Load",
          value: "5",
          icon: "fa fa-dashboard state-yellow"
        },
        {
          name: "High Load",
          value: "4",
          icon: "fa fa-dashboard state-red"
        }
      ],
      ram: [
        {
          name: "Low Usage",
          value: "43",
          icon: "fa fa-dashboard state-green"
        },
        {
          name: "Medium Usage",
          value: "5",
          icon: "fa fa-dashboard state-yellow"
        },
        {
          name: "High Usage",
          value: "4",
          icon: "fa fa-dashboard state-red"
        }
      ],
      disk: [
        {
          name: "Low Usage",
          value: "100",
          icon: "fa fa-dashboard state-green"
        },
        {
          name: "Medium Usage",
          value: "5",
          icon: "fa fa-dashboard state-yellow"
        },
        {
          name: "High Usage",
          value: "4",
          icon: "fa fa-dashboard state-red"
        }
      ]
    },

    template: templateEngine.createTemplate("healthView.ejs"),

    render: function () {
      $(this.el).html(this.template.render());
      this.updateView();
    },

    updateView: function() {
      _.each(this.dummyJSON, function(val, key) {

        var tds = "";

        _.each(val, function(v) {
          tds += '<tr><td>'+v.name+'</td>';
          tds += '<td>'+v.value+'</td>';
          tds += '<td><i class="'+v.icon+'"></i></td></tr>';
        });

        console.log(tds);

        jQuery('<tbody/>', {
          html: tds
        }).appendTo('.'+key+'-table');

      });
    }

  });
}());
