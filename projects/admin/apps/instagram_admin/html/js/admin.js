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
    
    var self = this;
    
    this.getQueuedImages(2, function() {
      
      // Approve / decline buttons events
      $('div#approved-btn').click(function(e) {
        if(!self.locked) {
          $(this).addClass('highlight');
          self.updateImage($('#instagram-images li:first-child'), true);
        }
      });

      $('div#declined-btn').click(function(e) {
        if(!self.locked) {
          $(this).addClass('highlight');
          self.updateImage($('#instagram-images li:first-child'), false);
        }
      });
      
    });
    
    
  }
  
  // 
  // --------------------------------------------------------
  ,getQueuedImages: function(limit, callback) {
    
    var self = this;
    
    // Get queued images json
    $.getJSON(
      'http://' 
      + this.options.http.host 
      + ':' 
      + this.options.http.port 
      + '/images/queued/' 
      + limit
      
      ,function(images) {
        
        // Add images
        self.addImages(images, callback);
      }
    );
  }
  
  // 
  // --------------------------------------------------------
  ,addImages: function(images, callback) {
    
    var check_img = function(queue, cb) {
      
      if(queue.length == 0) {
        cb();
        return;
      }
      
      var image = queue.pop();
      var url = image.images.low_resolution.url;
      
      var img = new Image();
      
      img.onload = function() { 

        var li = $('<li/>');
        var info = $('<div/>');
        info.addClass('instagram-image-info');

        info.html(image.user.username);

        li.append(info);

        li.attr({
          'id': 'instagram-image-' + image.media_id
        });

        li.data('media-id', image.media_id);
        li.data('item-id', image._id);

        li.css({
          'background-image': 'url(' + url + ')'
        });


        $('#instagram-images').append(li);
        
        check_img(queue, cb);

      };
      
      img.onerror = function() { 
        
        check_img(queue, cb);

      };
      
      img.src = url;
    };
    
    check_img($.merge(images, []), callback);
  }

  // 
  // --------------------------------------------------------
  ,updateImage: function(el, approved) {
    
    this.locked = true;
    
    var self = this;
    
    // Post new data to server
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
  
      self.getQueuedImages(1, function() {
        
        $('#instagram-images').fadeOut(150, function() {
          el.remove();
          $('#instagram-images').fadeIn(150, function() {
            self.locked = false; 
            $('.control-btn').removeClass('highlight');
          });
        });
      });
    })
  }
});

MosaicInstagramAdmin = new MosaicInstagramAdmin();