/************************************************************
 *
 * Mosaic Instagram Admin
 *
 ***********************************************************/

var MosaicInstagramAdmin = Class.extend({
  
  options: {
    http: {
      host: ''
      ,port: ''
    }
  }
  
  ,messageType: ''
  ,logoutTimer: null
  
  // Constructor
  // --------------------------------------------------------
  ,init: function(msgType, host, port) {
    
    var self = this;
    
    this.messageType = msgType;
    this.queue = [];
    
    this.options.http.host = host;
    this.options.http.port = port;
    
    $('#instagram-images-overlay').fadeOut(0);
    $('#controls').hide();
    
    // Logout
    
    this.logout_cb = function() {

      
      var last_id = $('#instagram-images li:first-child').data('item-id');
      if(last_id == undefined) { last_id = 0 };
      
      var queue_id = (self.queue.length > 0) ? self.queue[0].id : 0;
      var appr = (self.queue.length > 0) ? self.queue[0].approved : 0;

      $.get('/reset/' + last_id + '/' + queue_id + '/' + appr);
    }
    
    $('#logout-menu-link').click(function(e) {

      window.location.href = '/logout'; 
    });
    
    window.onbeforeunload = function() {
    
      self.logout_cb();
    }
    
    // Get new image
    this.getQueuedImages(this.messageType, 1, function() {
      
      // review callback
      var review_callback = function(btn, approved) {
        
        $('#instagram-images-result').hide();
        
        if(!self.locked) {
          
          self.locked = true;
          
          btn.addClass('highlight');
         
          if(self.queue.length > 0) {
            
            var img = self.queue.shift();
            var id = $('#instagram-images li:first-child').data('item-id');
            var url = $('#instagram-images li:first-child').data('media-url');
            var item = $('#instagram-images li:first-child').data('item');
            
            self.queue.push({
              id: id
              ,approved: approved
              ,url: url
              ,item: item
            });
            self.updateImage(img.id, img.approved, approved);
            
            return;
          }
          
          var el = $('#instagram-images li:first-child');
          $('#instagram-images-overlay div.inner-error').hide();
          $('#instagram-images-overlay div.inner-update').show();
          $('#instagram-images-overlay').fadeIn(100);
          
          $('#instagram-images-result').empty();
      
          if(approved) {  
            $('#instagram-images-result').css('background', 'green');
            $('#instagram-images-result').text('Approved');
          } else {
            $('#instagram-images-result').css('background', 'red');
            $('#instagram-images-result').text('Declined');
          }
          
          $('#instagram-images-result').show();
          
          self.queue.push({
            id: el.data('item-id')
            ,approved: true
            ,url: el.data('media-url')
            ,item: el.data('item')
          });
          
          setTimeout(function() {
          
            self.getQueuedImages(self.messageType, 1, function() {

              $('#instagram-images').fadeOut(100, function() {

                if($('#instagram-images li').length > 1 ) {
                  el.remove();
                }
                $('#instagram-images-overlay').fadeOut(100);
                $('#instagram-images').fadeIn(100, function() {
                  self.locked = false; 
                  $('.control-btn').removeClass('highlight');
                });
              });
            });
            
          }, 1000);
        }
      };
      
      // Approve / decline buttons events
      $('div#approved-btn').click(function(e) {
        review_callback($(this), true);
      });

      $('div#declined-btn').click(function(e) {
        review_callback($(this), false);
      });
      
    
      $('#instagram-images').hammer().bind('swiperight', function() {
        
        if(!self.locked) {
          if(self.queue.length > 0) {
            self.undoImage(); 
          }
        }
      });
      
      $(document).keyup(function(e) {
        
        // Undo (arrow up)
        if(e.keyCode == 37) {
          if(!self.locked) {
            if(self.queue.length > 0) {
              self.undoImage(); 
            }
          }
        }
        
        // Approve (arrow up)
        if(e.keyCode == 38) {
          $('div#approved-btn').addClass('highlight');
          review_callback($(this), true);
        }
        
        // Decline (arrow down)
        if(e.keyCode == 40) {
          
          if(!self.locked) {
            $('div#declined-btn').addClass('highlight');
            review_callback($(this), false);
          }
        }
      });
      
    });
  }
  
  // 
  // --------------------------------------------------------
  ,getQueuedImages: function(type, limit, callback) {
    
    var self = this;
    clearTimeout(self.logoutTimer);
    
    // Get queued images json
    $.getJSON(
      'http://' 
      + this.options.http.host 
      + ':' 
      + this.options.http.port 
      + '/images/queued/' 
      + type
      + '/0/0/'
      + limit
    ).done(function(images) {
        
        // Check for images
        if(images.length == 0) {
          
          $('#controls').hide();
          $('#instagram-images').fadeOut(100, function() {
           
            self.retryRequest(type, limit, function() {
            
              $('#instagram-images-overlay').fadeOut(100);
              $('#instagram-images').fadeIn(100);
              callback();
            });
          });
          
          
          return;
        }
        
        $('#controls').show();
        
        // Add images
        self.addImages(images, callback);
      }
    ).error(function() {
      
      $('#controls').hide();
      $('#instagram-images').fadeOut(100, function() {
        
        $('#instagram-images').empty();
        self.retryRequest(type, limit, function() {

          $('#instagram-images-overlay').fadeOut(100);
          $('#instagram-images').fadeIn(100);
          callback();
        });
      });
    });
  }
  
  // 
  // --------------------------------------------------------
  ,undoImage: function() {
    
    this.addImages([this.queue[0].item], function() {
      
      $('#instagram-images').fadeOut(100, function() {
          
        if($('#instagram-images li').length > 1 ) {
          $('#instagram-images li:first-child').remove();
        }
        $('#instagram-images-overlay').fadeOut(100);
        $('#instagram-images').fadeIn(100, function() {
          self.locked = false; 
          $('.control-btn').removeClass('highlight');
        });
      });
    });
   
  }
  
  // 
  // --------------------------------------------------------
  ,retryRequest: function(type, limit, callback) {
    
    var self = this;

    $('#instagram-images-overlay div.inner-update').hide();
    $('#instagram-images-overlay div.inner-error').show();
    $('#instagram-images-overlay').fadeIn(100, function() {
      
      var update_seconds = function(secondsLeft) {
        
        if(secondsLeft == 0) {
          self.getQueuedImages(type, limit, callback);
          return;
        }
        
        $('#error-timeout').text('Retry in '+secondsLeft+' seconds.');
        secondsLeft--;
        
        setTimeout(function() {
          update_seconds(secondsLeft);
        }, 1000);
      }
      
      update_seconds(5);
    });
  }
  
  // 
  // --------------------------------------------------------
  ,addImages: function(images, callback) {
    
    var self = this;
    
    var check_img = function(queue, cb) {
      
      if(queue.length == 0) {
        
        clearTimeout(self.logoutTimer);
        self.logoutTimer = setTimeout(function() {
          window.location.href = '/logout'; 
        }, 600000);
        
        self.locked = false;
        cb();
        return;
      }
      
      var image = queue.pop();
      var url = image.images.low_resolution.url;
      
      var img = new Image();
      var img_loaded = false;
      
      img.onerror = function() { 
        
        if(img_loaded) return;
        
        $('#error-timeout').text('Please wait while we look for new images...');
        $('#instagram-images-overlay div.inner-error').show();
        $('#instagram-images-overlay div.inner-update').hide();
        
        $('#instagram-images-overlay').fadeIn(0, function() {
          
          setTimeout(function() {
            
            self.getQueuedImages(self.messageType, 1, function() {
          
              $('#instagram-images-overlay').hide();
              cb();
            });
            
          }, 1500);
        });

      };
      
      img.onload = function() { 
        
        img_loaded = true;
        
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
        li.data('media-url', image.images.low_resolution.url);
        li.data('item', image);

        li.css({
          'background-image': 'url(' + url + ')'
        });


        $('#instagram-images').append(li);
        
        check_img(queue, cb);

      };

      img.src = url;
    };
    
    check_img($.merge(images, []), callback);
  }

  // 
  // --------------------------------------------------------
  ,updateImage: function(id, approved, currentApproved) {
  
    var self = this;
    $('#instagram-images-result').hide();
    $('#instagram-images-overlay div.inner-error').hide();
    $('#instagram-images-overlay div.inner-update').show();
    $('#instagram-images-overlay').fadeIn(100);
    
    // Post new data to server
    $.post(
      'http://' 
      + this.options.http.host 
      + ':' 
      + this.options.http.port 
      + '/images/update/' 
      ,{
        id: id
        ,approved: approved
      }).done(function() {
      
      $('#instagram-images-result').empty();
      
      if(currentApproved) {  
        $('#instagram-images-result').css('background', 'green');
        $('#instagram-images-result').text('Approved');
      } else {
        $('#instagram-images-result').css('background', 'red');
        $('#instagram-images-result').text('Declined');
      }
      
      $('#instagram-images-result').show();
  
      self.getQueuedImages(self.messageType, 1, function() {
        
        $('#instagram-images').fadeOut(100, function() {
          
          if($('#instagram-images li').length > 1 ) {
            $('#instagram-images li:first-child').remove();
          }
          $('#instagram-images-overlay').fadeOut(100);
          $('#instagram-images').fadeIn(100, function() {
            self.locked = false; 
            $('.control-btn').removeClass('highlight');
          });
        });
      });
    }).error(function() {
      
      self.retryRequest(self.messageType, 1, function() {
        
        if($('#instagram-images li').length > 1 ) {
          $('#instagram-images li:first-child').remove();
        }
        $('#instagram-images-overlay').fadeOut(100);
        $('#instagram-images').fadeIn(100, function() {
          self.locked = false; 
          $('.control-btn').removeClass('highlight');
        });
      });
    });
  }
});
