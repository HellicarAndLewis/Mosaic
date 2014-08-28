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
  }
  
  // 
  // --------------------------------------------------------
  ,getQueuedImages: function(limit) {
    
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
      }
    );
  }
  
  // 
  // --------------------------------------------------------
  ,addImages: function(images) {
    
    $(images).each(function(i, image) {
      
      var li = $('<li/>');
      var img = new Image();

      img.src = image.images.thumbnail.url;
      $(img).attr('width', '100%');
      $(img).attr('height', '100%');
      
      li.append($(img));
      
      $('#instagram-images').append(li);
    });
  }

  // 
  // --------------------------------------------------------
  ,updateImage: function(id, approved) {
    
    $.post(
      'http://' 
      + this.options.http.host 
      + ':' 
      + this.options.http.port 
      + '/images/update/' 
      ,{
        id: id
        ,approved: approved
      })
    
    .done(function( data ) {
      alert( "Data Loaded: " + data );
    })
  }
});

MosaicInstagramAdmin = new MosaicInstagramAdmin();