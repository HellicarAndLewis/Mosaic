/************************************************************
 *
 * Mosaic Instagram Admin
 *
 ***********************************************************/

var MosaicInstagramAdmin = Class.extend({
  
  options: {
    http: {
      host: 'localhost'
      ,port: '3333'
    }
  }
  
  // Constructor
  // --------------------------------------------------------
  ,init: function() {
    
    // Setup socket.io
    var socket = io.connect('http://localhost:3333');
    
    this.getQueuedImages(2);
    
    var self = this;
    $('div#approved-btn').click(function(e) {
      self.updateImage($('#instagram-images li:first-child'), true);
    });
    
    $('div#declined-btn').click(function(e) {
      self.updateImage($('#instagram-images li:first-child'), false);
    });
  }
  
  // 
  // --------------------------------------------------------
  ,getQueuedImages: function(limit, callback) {
    
    var self = this;
  
    $.getJSON(
      'http://' 
      + this.options.http.host 
      + ':' 
      + this.options.http.port 
      + '/images/queued/' 
      + limit
      
      ,function(images) {
      
        self.addImages(images);
        
        if(callback) {
          callback(); 
        }
      }
    );
  }
  
  // 
  // --------------------------------------------------------
  ,addImages: function(images) {
    
    $(images).each(function(i, image) {
      
      var li = $('<li/>');
      
      li.attr({
        'id': 'instagram-image-' + image.id
      });
      
      li.data('media-id', image.id);
      li.data('item-id', image._id);
      
      li.css({
        'background-image': 'url(' + image.images.low_resolution.url + ')'
      });
      
      
      $('#instagram-images').append(li);
    });
  }

  // 
  // --------------------------------------------------------
  ,updateImage: function(el, approved) {
    
    var self = this;
    
    $.post(
      'http://' 
      + this.options.http.host 
      + ':' 
      + this.options.http.port 
      + '/images/update/' 
      ,{
        id: el.data('item-id')
        ,approved: approved
      }).done(function(data) {
  
      self.getQueuedImages(2, function() {
       el.remove(); 
      });
    })
  }
});

MosaicInstagramAdmin = new MosaicInstagramAdmin();