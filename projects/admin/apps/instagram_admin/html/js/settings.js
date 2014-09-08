/************************************************************
 *
 * Mosaic Instagram Admin Settings
 *
 ***********************************************************/

var MosaicInstagramAdminSettings = Class.extend({
  
  options: {
    http: {
      host: ''
      ,port: ''
    }
  }
  
  // Constructor
  // --------------------------------------------------------
  ,init: function(showMosaic, host, port) {

    var self = this;  
    
    this.options.http.host = host;
    this.options.http.port = port;
  
    $('.toggle').toggles({
      height: 30
    }); 

    // Show mosaic toggle
    $('#toggle-show-mosaic').toggles(showMosaic);
    $('#toggle-show-mosaic').on('toggle', function (e, active) {

      $.post(
        'http://' 
        + self.options.http.host 
        + ':' 
        + self.options.http.port 
        + '/settings/update' 
        ,{
          show_mosaic: active
        }, function() {
         
        });

    });

  }
});
