/*global beforeEach, afterEach */
/*global describe, it, expect, jasmine, spyOn*/
/*global waitsFor, runs, waits */
/*global _, JSHINT*/
/*global document*/

(function () {
  "use strict";
  describe('JSLint', function () {
    var options = {
          unused: false, // should be `true`
          strict: false, // should be `true`
          bitwise: true,
          curly: true,
          eqeqeq: true,
          forin: true,
          freeze: true,
          immed: true,
          laxbreak: true,
          newcap: true,
          noarg: true,
          noempty: true,
          nonbsp: true,
          nonew: true,
          undef: true,
          indent: 2,
          maxlen: 120
        },
        lint = /^\/specJSLint|[\w\W]jsLintSpec\.js$/;

    function get(path) {
      path = path + "?" + new Date().getTime();

      var xhr;
      try {
        xhr = new jasmine.XmlHttpRequest();
        xhr.open("GET", path, false);
        xhr.send(null);
      } catch (e) {
        throw new Error("couldn't fetch " + path + ": " + e);
      }
      if (xhr.status < 200 || xhr.status > 299) {
        throw new Error("Could not load '" + path + "'.");
      }

      return xhr.responseText;
    }

    describe('checking codeFiles', function() {
     var files = /^([\w\W]*lib\/[\w\W]*)|([\w\W]*Spec\.js)$/,
       generated = /(\/bootstrap\/)|(\/client\/)|(\/modules\/)|(\/shell\/)/,
       ace = /ace\.js/;

      _.each(document.getElementsByTagName('script'), function (element) {
        var script = element.getAttribute('src');
        if (script === null || files.test(script) || generated.test(script) || ace.test(script)) {
          return;
        }
        it(script, function () {
          var self = this,
            source = get(script),
            result = JSHINT(source, options);
          _.each(JSHINT.errors, function (error) {
            self.addMatcherResult(new jasmine.ExpectationResult({
              passed: false,
              message: "line " + error.line + ' - ' + error.reason + ' - ' + error.evidence
            }));
          });
          expect(true).toBe(true); // force spec to show up if there are no errors
        });
      });
    });

    describe('checking specFiles', function() {
      var files = /^\/spec*|[\w\W]*Spec\.js$/;

      _.each(document.getElementsByTagName('script'), function (element) {
        var script = element.getAttribute('src');
        if (!files.test(script) || lint.test(script)) {
          return;
        }
        it(script, function () {
          var self = this,
            source = get(script),
            result = JSHINT(source, options);
          _.each(JSHINT.errors, function (error) {
            self.addMatcherResult(new jasmine.ExpectationResult({
              passed: false,
              message: "line " + error.line + ' - ' + error.reason + ' - ' + error.evidence
            }));
          });
          expect(true).toBe(true); // force spec to show up if there are no errors
        });
      });
    });
  });
}());
