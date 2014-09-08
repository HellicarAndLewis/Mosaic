/************************************************************
 *
 * Mosaic Instagram Admin Settings
 *
 ***********************************************************/

var MosaicInstagramAdminSettings = Class.extend({
  
  // Constructor
  // --------------------------------------------------------
  init: function() {
    
    $(document).ready(function() {
      $('.toggle').toggles({
        height: 30
      });  
    });
  }
});

MosaicInstagramAdminSettings = new MosaicInstagramAdminSettings();
