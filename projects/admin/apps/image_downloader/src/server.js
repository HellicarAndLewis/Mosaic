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
var Exec = require('child_process').exec;

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
      .option('-c, --compare', 'Compare db with local files')
      .parse(process.argv);
    
    
    this.lastModIdMin = 0;
    this.lastModIdMax = 0;
    
    var file = './settings.json';
    
    if(Program.settings) {
      file = Path.normalize(process.cwd() + '/' + Program.settings);
    }

    var start_download = function() {

        if(Program.debug) { self.settings.debug = true; }

        self.log('Started with settings ' + file);


        // Check if tmp dir exists
        Fs.ensureDir(self.settings.image_tmp_path, function(err) {

          self.log('Checking tmp dir ' + self.settings.image_tmp_path);

          if(err) {
            self.log('Tmp dir ' + self.settings.image_tmp_path + 'is not ok');
          }

          // Check if raw tags save dir exists
          Fs.ensureDir(self.settings.image_raw_path_tags, function(err) {

            self.log('Checking raw tags save dir ' + self.settings.image_raw_path_tags);

            if(err) {
              self.log('Raw tags save dir ' + self.settings.image_raw_path_tags + 'is not ok');
            }
            
            // Check if raw users save dir exists
            Fs.ensureDir(self.settings.image_raw_path_users, function(err) {

              self.log('Checking raw users save dir ' + self.settings.image_raw_path_users);

              if(err) {
                self.log('Raw users save dir ' + self.settings.image_raw_path_users + 'is not ok');
              }


              // Start downloading!
              self.start();

            }); 
 
          }); 
        });
    }
  
    // Check if file exists...
    if(Fs.existsSync(file)) {

      // Include project file
      self.settings = require(file);

      // Start
      start_download(); 

    } else {

      // No file found
      self.log('Could not find settings file at ' + file);
      process.exit(0);
    }

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

          setTimeout(function() {
              
            // Retry  
            self.lastModIdMin = 0;
            self.lastModIdMax = 0;
            self.start();
            
          }, self.settings.request_delay);
          
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
          var dest_file = self.settings.image_raw_path_tags + img.media_id + '.jpg';
          
          if(img.msg_type == 'user') {
            dest_file = self.settings.image_raw_path_users + img.media_id + '.jpg';
          }
          
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
          
          
          // Check if file exists
          var file_exists = (current_images.indexOf(img.media_id) >= 0);
       
          // ..if not start downloading
          if(!file_exists) {
            
            // Download image
            //self.log('Trying to download ' + img.images[self.settings.image_size_download].url);
            
            var next_img = function() {
              // Next
              setTimeout(function() {
                dl_img(queue);
              }, self.settings.image_save_delay); 
            };
            
            self.download(img.images['standard_resolution'].url, tmp_file, function(err) {
            
              // Error
              if(err || !Fs.existsSync(tmp_file)) {
                next_img();
                return;
              }
              
              var stats = Fs.statSync(tmp_file)
              var size = stats['size'];
  
              // Invalid image size
              if(size < 1000) {
                
                // Remove invalid image
                Fs.unlink(tmp_file, function(err) {
                  
                  // Error
                  if(err) {
                    next_img();
                    return;
                  }
                  
                  self.log('Image size invalid. ' + size + ' bytes received, removed tmp file ' + tmp_file);
                  
                  // Next
                  next_img();
                });
              
              
              // Valid size
              } else {
                
                Fs.outputJson(self.settings.image_json_path + img.media_id + '.json', {user:img.user,tags:img.tags}, function(err) {
                  
                  // Error
                  if(err || !Fs.existsSync(tmp_file)) {
                    next_img();
                    return;
                  }
                  
                  // Move image from tmp to save dir
                  Fs.copy(tmp_file, dest_file, function(err) {
                    
                    // Error
                    if(err || !Fs.existsSync(tmp_file)) {
                      next_img();
                      return;
                    }
                    
                    // Remove tmp file
                    Fs.unlink(tmp_file, function(err) {
                      
                      // Error
                      if(err) {
                        next_img();
                        return;
                      }

                      // Add to images db
                      current_images.push(img.media_id);

                      // Update images db
                      Fs.outputJson(self.settings.images_db, current_images, function(err) {

                        self.log('Image ' + dest_file + ' saved');

                        // Next
                        next_img();
                      });
                    });
                  });
                  
                });
              }
              
            }, function() {
              
              // Next
              next_img();
            });
            
          } else {

            dl_img(queue);
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
    
    if(this.settings) {
      if(this.settings.debug) {
        console.log(s);
      }
    }
  }
  
  // 
  // --------------------------------------------------------
  ,download:function(uri, filename, callback, errCallback) {
    
    // Start download request
    Request.head(uri, function(err, res, body) {
      
      try {
        var r = Request(uri).pipe(Fs.createWriteStream(filename));
        r.on('close', callback);
      } catch(err) {
        callback(err);
      }
    });
  }
});

var ImageDownloader = new ImageDownloader();