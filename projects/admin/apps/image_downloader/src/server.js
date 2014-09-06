/************************************************************
 *
 * Image downloader
 *
 ***********************************************************/

require('mootools');

var Fs = require('fs-extra');
var Request = require('request');
var Program = require('commander');
var Path = require('path');
var Gm = require('gm');

var ImageDownloader = new Class({
  
  // Constructor
  // --------------------------------------------------------
  initialize: function() {
    
    var self = this;
    
    // Process args
    this.args = Program
      .version('0.0.1')
      .option('-s, --settings [path]', 'Start with custom settings file')
      .option('-d, --debug', 'Force debug console output')
      .option('-h, --homedir', 'Use homedir as base folder')
      .option('-c, --compare', 'Compare db with local files')
      .parse(process.argv);
    
    
    this.lastModIdMin = 0;
    this.lastModIdMax = 0;
    
    var file = './settings.json';
    
    if(Program.settings) {
      file = Path.normalize(process.cwd() + '/' + Program.settings);
    }
    
    // TEMP
    Fs.remove('/Users/martinbartels/Sites/mosaic/projects/admin/apps/image_downloader/downloads/images/', function() {

    // Check if file exists...
    if(Fs.existsSync(file)) {
    
      // Include project file
      self.settings = require(file);
      
      if(Program.debug) { this.settings.debug = true; }
      
      self.log('Started with settings ' + file);
      
      if(Program.homedir) {
        
        self.settings.image_tmp_path = Path.normalize(process.env['HOME'] + '/' + self.settings.image_tmp_path);
        self.settings.image_save_path_users = Path.normalize(process.env['HOME'] + '/' + self.settings.image_save_path_users);
        self.settings.image_save_path_tags = Path.normalize(process.env['HOME'] + '/' + self.settings.image_save_path_tags);
      }
      
      // Check if tmp dir exists
      Fs.ensureDir(self.settings.image_tmp_path, function(err) {
        
        self.log('Checking tmp dir ' + self.settings.image_tmp_path);
        
        if(err) {
          self.log('Tmp dir ' + self.settings.image_tmp_path + 'is not ok');
        }
        
        // Check if save dir exists
        Fs.ensureDir(self.settings.image_save_path_users, function(err) {

          self.log('Checking users dir ' + self.settings.image_save_path_users);

          if(err) {
            self.log('Users dir ' + self.settings.image_save_path_users + 'is not ok');
          }
          
          Fs.ensureDir(self.settings.image_save_path_tags, function(err) {

            self.log('Checking tags dir ' + self.settings.image_save_path_tags);

            if(err) {
              self.log('Tags dir ' + self.settings.image_save_path_tags + 'is not ok');
            }
            
            // Resize assets
            
            
            self.thumbFilename = self.settings.image_overlay_thumb 
              + '.' 
              + self.settings.image_size_thumb.width 
              + 'x' 
              + self.settings.image_size_thumb.height 
              + '.png';
            
            self.largeFilename = self.settings.image_overlay_large 
              + '.' 
              + self.settings.image_size_large.width 
              + 'x' 
              + self.settings.image_size_large.height 
              + '.png';
            
            // .. thumb
            Gm(self.settings.image_overlay_thumb)
              .resize(self.settings.image_size_thumb.width, self.settings.image_size_thumb.height, '!')
              .colors(256)
              .write(self.thumbFilename, function(err) {
            
                // .. large
                Gm(self.settings.image_overlay_large)
                  .resize(self.settings.image_size_large.width, self.settings.image_size_large.height, '!')
                  .write(self.largeFilename, function(err) {

                    // Start downloading!
                    self.start();
                  });
                
              });
          });  
        }); 
      });
 
    } else {
    
      // No file found
      self.log('Could not find settings file at ' + file);
      process.exit(0);
    }

    }); // TEMP
  }
  
  // 
  // --------------------------------------------------------
  ,start: function() {
    
    var self = this;
    var url = this.settings.queue_url + '/all/' +  this.lastModIdMin + '/' + this.lastModIdMax + '/' + this.settings.limit + '/';

    
    self.log('Get new results - ' + url);
    self.rechecked = false;
    
    clearTimeout(self.reqTimeout);
    
    self.reqTimeout = setTimeout(function() {
            
      self.lastModIdMin = 0;
      self.lastModIdMax = 0;
      self.start();
      
    }, self.settings.request_timeout_restart);

    // Start request
    Request({
      url: url
      ,json: true
    }
    ,function(error, response, images) {
      
      // No error
      if(!error) {
        
        // Filter images
        images.filter(function(image, i) {
          
          return (!image.locked && (image.reviewed && image.approved));
        });
        
        // Check for images
        if(images.length > 0) {
          
          // Use pagination
          if(Program.compare) {
            
            self.lastModIdMin = images[images.length-1].queue_id;
          }
        
        // .. no images
        } else {
          
          // Retry  
          self.lastModIdMin = 0;
          self.lastModIdMax = 0;
          self.start();
          
          return;
        }
        
        // Paginantion
        if(self.lastModIdMax == 0 && Program.compare) {
          
          self.lastModIdMax = images[0].queue_id; 
        }
        
        // Download callback
        var dl_img = function(queue) {
          
          if(queue.length == 0) {
            
            setTimeout(function() {
              
              self.start();
            }, self.settings.request_delay);
            return;
          }
          
          // Pop last image
          var img = queue.pop();
          
          // Set all paths
          // -----------------------------------------------------------------------------
          var tmp_file = self.settings.image_tmp_path + img.media_id + '.jpg';
          
          var dest_file_users = self.settings.image_save_path_users + img.media_id + '.jpg';
          var dest_file_tags = self.settings.image_save_path_tags + img.media_id + '.jpg';
          var dest_file_users_json = self.settings.image_save_path_users_json + img.media_id + '.json';
          var dest_file_tags_json = self.settings.image_save_path_tags_json + img.media_id + '.json';
          
          var thumb_dir = (img.msg_type == 'tag') ? self.settings.image_save_path_tags_thumb : self.settings.image_save_path_users_thumb;
          var thumb_file = thumb_dir + img.media_id + '.jpg';
          
          var large_dir = (img.msg_type == 'tag') ? self.settings.image_save_path_tags_large : self.settings.image_save_path_users_large;
          var large_file = large_dir + img.media_id + '.jpg';
          
          var dest_file = (img.msg_type == 'tag') ? dest_file_tags : dest_file_users;
          var json_file = (img.msg_type == 'tag') ? dest_file_tags_json : dest_file_users_json;
          
          // -----------------------------------------------------------------------------
          
          // Current downloaded images
          var current_images = [];
          if(Fs.existsSync(self.settings.images_db)) {
            current_images = require(self.settings.images_db);
          }
          
          // Reset images db
          if(current_images.length > (self.settings.limit * 2)) {
            current_images.shift();
          }
          
          console.log(current_images.length);
          
          // Check if file exists
          var file_exists = (current_images.indexOf(img.media_id) >= 0);
       
          // ..if not start downloading
          if(!file_exists) {
            
            // Download image
            //self.log('Trying to download ' + img.images[self.settings.image_size_download].url);
            
            self.download(img.images[self.settings.image_size_download].url, tmp_file, function(err) {
              
              var stats = Fs.statSync(tmp_file)
              var size = stats["size"];
              
              // Invalid image size
              if(size < 1000) {
                
                // Remove invalid image
                Fs.unlink(tmp_file, function() {
                  
                  self.log('Image size invalid. ' + size + ' bytes received, removed tmp file ' + tmp_file);
                  
                  // Next
                  setTimeout(function() {
                    dl_img(queue);
                  }, self.settings.image_save_delay);
                });
              
              
              // Valid size
              } else {
                
                  
                var create_large = function() {
                    
                  // Create large
                  Fs.ensureDir(large_dir, function(err) {

                    // Image processing large
                    var ix = self.settings.image_size_large.inner.x;
                    var iy = self.settings.image_size_large.inner.y;
                    var tw = self.settings.image_size_large.width;
                    var th = self.settings.image_size_large.height;
                    var tiw = self.settings.image_size_large.inner.width;
                    var tih = self.settings.image_size_large.inner.height;

                    Gm(tmp_file)
                      .resize(tw, th)
                      .crop(tiw, tih, (tw-tiw)/2, (th-tih)/2)
                      .write(large_file, function(err) {

                        Gm()
                        .in('-page', '+0+0')
                        .in(self.largeFilename)
                        .in('-page', '+'+ix+'+'+iy)
                        .in(large_file)
                        .in('-flatten')
                        .write(large_file, function(err) {

                          // Add text
                          Gm(large_file)
                            .font(self.settings.image_font)
                            .fontSize(self.settings.image_size_large.text.username.font_size)
                            .drawText(
                              self.settings.image_size_large.text.username.x
                              ,-(th/2) + self.settings.image_size_large.text.username.y
                              ,img.user.username.toUpperCase(), 'center')
                            .fontSize(self.settings.image_size_large.text.hashtag.font_size)
                            .drawText(
                              self.settings.image_size_large.text.hashtag.x
                              ,-(th/2) + self.settings.image_size_large.text.hashtag.y
                              ,self.settings.image_size_large.text.hashtag.text
                              , 'center'
                            )
                            .write(large_file, function(err) {

                              // Move image from tmp to save dir
                              Fs.copy(tmp_file, dest_file, function() {

                                // Remove tmp file
                                Fs.unlink(tmp_file, function() {

                                  // Add to images db
                                  current_images.push(img.media_id);

                                  // Update images db
                                  Fs.outputJson(self.settings.images_db, current_images, function(err) {

                                    self.log('Image ' + dest_file + ' saved');

                                    // Next
                                    setTimeout(function() {

                                      dl_img(queue);
                                    }, self.settings.image_save_delay);
                                  });
                                });
                              });
                            });

                        });
                      });
                  });
                }
                
                // Create thumbnail
                Fs.ensureDir(thumb_dir, function(err) {

                  // Image processing thumb
                  var ix = self.settings.image_size_thumb.inner.x;
                  var iy = self.settings.image_size_thumb.inner.y;
                  var tw = self.settings.image_size_thumb.width;
                  var th = self.settings.image_size_thumb.height;
                  var tiw = self.settings.image_size_thumb.inner.width;
                  var tih = self.settings.image_size_thumb.inner.height;

                  Gm(tmp_file)
                    .resize(tw, th)
                    .crop(tiw, tih, (tw-tiw)/2, (th-tih)/2)
                    .write(thumb_file, function(err) {

                      Gm()
                      .in('-page', '+0+0')
                      .in(self.thumbFilename)
                      .in('-page', '+'+ix+'+'+iy)
                      .in(thumb_file)
                      .in('-flatten')
                      .write(thumb_file, function(err) {

                        // Add text
                        Gm(thumb_file)
                          .font(self.settings.image_font)
                          .fontSize(self.settings.image_size_thumb.text.username.font_size)
                          .drawText(
                            self.settings.image_size_thumb.text.username.x
                            ,-(th/2) + self.settings.image_size_thumb.text.username.y
                            ,img.user.username.toUpperCase(), 'center')
                          .fontSize(self.settings.image_size_thumb.text.hashtag.font_size)
                          .drawText(
                            self.settings.image_size_thumb.text.hashtag.x
                            ,-(th/2) + self.settings.image_size_thumb.text.hashtag.y
                            ,self.settings.image_size_thumb.text.hashtag.text
                            , 'center'
                          )
                          .write(thumb_file, function(err) {

                            create_large();
                          });

                      });
                    });
                });

              }
              
            }, function() {
              
              setTimeout(function() {
                dl_img(queue);
              }, self.settings.image_save_delay);
            });
          } else {
            
            setTimeout(function() {
              dl_img(queue);
            }, self.settings.image_save_delay);
          }
        };
        
        dl_img(Array.clone(images));
        
      } else {
        
        self.log('Failed to load ' + url);
        
        
        
        setTimeout(function() {
          
          self.lastModIdMin = 0;
          self.lastModIdMax = 0;
          self.start();
          
        }, self.settings.request_delay);
        return;
      }
    });
  }
  
  ,log: function(s) {
    
    if(this.settings.debug) {
      console.log(s);
    }
  }
  
  // 
  // --------------------------------------------------------
  ,download:function(uri, filename, callback, errCallback) {
    
    // Start download request
    Request.head(uri, function(err, res, body) {

      var r = Request(uri).pipe(Fs.createWriteStream(filename));
      r.on('close', callback);
    });
  }
});

var ImageDownloader = new ImageDownloader();