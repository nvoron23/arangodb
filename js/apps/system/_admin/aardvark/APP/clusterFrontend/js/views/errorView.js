/*jshint browser: true */
/*jshint unused: false */
/*global Backbone, templateEngine, $, window, arangoHelper*/
(function () {
  "use strict";
  window.ErrorView = Backbone.View.extend({

    events: {
    },

    template: templateEngine.createTemplate("errorView.ejs"),

    show: function (errorMsg) {

      if ($('.errorNavbar')) {
        $('.errorNavbar').remove();
      }

      if (errorMsg) {
        $('.clusterNavbar ul').before(this.template.render({message: errorMsg}));
        $('.health-menu').addClass('error');
        $('.health-menu').addClass('border-top');
      }
    },

    hide: function () {
      $('.errorNavbar').remove();
      $('.health-menu').removeClass('error');
      $('.health-menu').removeClass('border-top');
    }

  });
}());
