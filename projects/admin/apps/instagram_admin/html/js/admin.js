var MosaicInstagramAdmin = Class.extend({

  init: function() {
    
    // Setup socket.io
    var socket = io.connect('http://localhost:3333');
    
    // Media callback
    socket.on('medias', function(data) {
      
      if(data) {
     
        $.each(data, function(j, dataItem) {
         
          var li = $('<li/>');
          li.attr('id', dataItem.id);
          
          var img = new Image();
          img.src = dataItem.images.thumbnail.url;
         
          li.append($(img));
          $('#instagram-images').append(li);
        });
    
      }
      
    });
  }
});

MosaicInstagramAdmin = new MosaicInstagramAdmin();