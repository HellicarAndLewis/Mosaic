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
    
    this.options.http.host = host;
    this.options.http.port = port;
    
    $('#instagram-images-overlay').fadeOut(0);
    $('#controls').hide();
    
    // Logout
    $('#logout-menu-link').click(function(e) {

      e.preventDefault();
      var last_id = $('#instagram-images li:first-child').data('item-id');
      if(last_id == undefined) { last_id = 0 };
      window.location.href = '/logout/' + last_id;
    });
    
    this.getQueuedImages(this.messageType, 1, function() {
      
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
      
      // Touch
      /*
      // Approve
      $('#instagram-images').hammer().bind('swipeleft', function() {
        
        if(!self.locked) {
          $('div#approved-btn').addClass('highlight');
          self.updateImage($('#instagram-images li:first-child'), true);
        }
      });
      
      // Decline
      $('#instagram-images').hammer().bind('swiperight', function() {
        
        if(!self.locked) {
          $('div#declined-btn').addClass('highlight');
          self.updateImage($('#instagram-images li:first-child'), false);
        }
      });
      */
      
      $(document).keyup(function(e) {
        
        // Approve (arrow up)
        if(e.keyCode == 38) {
          
          if(!self.locked) {
            $('div#approved-btn').addClass('highlight');
            self.updateImage($('#instagram-images li:first-child'), true);
          }
        }
        
        // Decline (arrow down)
        if(e.keyCode == 40) {
          
          if(!self.locked) {
            $('div#declined-btn').addClass('highlight');
            self.updateImage($('#instagram-images li:first-child'), false);
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
            $('#instagram-images').empty();
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
        
        $('#error-timeout').text(secondsLeft);
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
        var last_id = $('#instagram-images li:first-child').data('item-id');
        if(last_id == undefined) { last_id = 0 };
        
        self.locked = false;
        
        self.logoutTimer = setTimeout(function() {
          window.location.href = '/logout/' + last_id;
        }, 60000);
        
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
        
        self.retryRequest(self.messageType, 1, function() {
        
          $('#instagram-images').fadeOut(100, function() {
        
            $('#instagram-images-overlay').fadeOut(100);
            $('#instagram-images').fadeIn(100, function() {
              self.locked = false; 
              $('.control-btn').removeClass('highlight');
            });
          });
        });

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
        id: el.data('item-id')
        ,approved: approved
      }).done(function() {
      
      $('#instagram-images-result').empty();
      
      if(approved) {  
        $('#instagram-images-result').css('background', 'green');
        $('#instagram-images-result').text('Approved');
      } else {
        $('#instagram-images-result').css('background', 'red');
        $('#instagram-images-result').text('Declined');
      }
  
      self.getQueuedImages(self.messageType, 1, function() {
        
        $('#instagram-images').fadeOut(100, function() {
          
          el.remove();
          $('#instagram-images-overlay').fadeOut(100);
          $('#instagram-images').fadeIn(100, function() {
            self.locked = false; 
            $('.control-btn').removeClass('highlight');
          });
        });
      });
    }).error(function() {
      
      self.retryRequest(self.messageType, 1, function() {
        
        el.remove();
        $('#instagram-images-overlay').fadeOut(100);
        $('#instagram-images').fadeIn(100, function() {
          self.locked = false; 
          $('.control-btn').removeClass('highlight');
        });
      });
    });
  }
});
