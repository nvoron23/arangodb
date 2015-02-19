/*jshint browser: true */
/*jshint unused: false */
/*global Backbone, templateEngine, $, window, arangoHelper*/
(function () {
  "use strict";
  window.NavigationView = Backbone.View.extend({
    el: '#navigationBar',

    events: {
    },

    initialize: function () {
      jQuery('<div/>', {
        class: 'clusterNavbar'
      }).appendTo('.navbar .resizecontainer');

      this.subNavigation = new window.EmbeddedNavigationView();
      this.subNavigation.render();
    },

    template: templateEngine.createTemplate("clusterNavigationView.ejs"),

    render: function () {
      $(this.el).html(this.template.render());
    },

    selectMenuItem: function (menuItem) {
      $('.navlist li').removeClass('active');
      if (menuItem) {
        $('.' + menuItem).addClass('active');
      }
    }


  });
}());
