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
      .parse(process.argv);
    
    
    this.lastModIdMin = 0;
    this.lastModIdMax = 0;
    
    var file = './settings.json';
    
    if(Program.settings) {
      file = Path.normalize(process.cwd() + '/' + Program.settings);
    }
    
    
    
    // Check if file exists...
    if(Fs.existsSync(file)) {
    
      // Include project file
      self.settings = require(file);
      
      if(Program.debug) { this.settings.debug = true; }
      
      self.log('Started with settings ' + file);
      
      self.settings.image_tmp_path = Path.normalize(process.env['HOME'] + '/' + self.settings.image_tmp_path);
      self.settings.image_save_path_users = Path.normalize(process.env['HOME'] + '/' + self.settings.image_save_path_users);
      self.settings.image_save_path_tags = Path.normalize(process.env['HOME'] + '/' + self.settings.image_save_path_tags);
      
      // Check if tmp dir exists
      Fs.ensureDir(self.settings.image_tmp_path, function() {
        self.log('Checking tmp dir ' + self.settings.image_tmp_path);
      });

      // Check if save dir exists
      Fs.ensureDir(self.settings.image_save_path_users, function(err) {
       
        self.log('Checking users dir ' + self.settings.image_save_path_users);
      });
      
      Fs.ensureDir(self.settings.image_save_path_tags, function() {
        self.log('Checking tags dir ' + self.settings.image_save_path_tags);
      });
      
      
      
      
      self.start();
      
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

    // Start request
    Request({
      url: url
      ,json: true
    }
    ,function(error, response, images) {
      
      // No error
      if(!error) {
        
        images.filter(function(image, i) {
          return (!image.locked && (image.reviewed && image.approved));
        });

        if(images.length > 0) {
          self.lastModIdMin = images[images.length-1].queue_id;
        } else {
          setTimeout(function() {
            self.start();
          }, self.settings.request_delay);
          return;
        }
        
        if(self.lastModIdMax == '0') {
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
          
          var tmp_file = self.settings.image_tmp_path + img._id + '.jpg';
          var dest_file_users = self.settings.image_save_path_users + img._id + '.jpg';
          var dest_file_tags = self.settings.image_save_path_tags + img._id + '.jpg';
          var dest_file = (img.msg_type == 'tag') ? dest_file_tags : dest_file_users;
          self.log(dest_file);
          var file_exists = Fs.existsSync(dest_file);
          if(!file_exists) {
            
            // Download image
            self.download(img.images[self.settings.image_size].url, tmp_file, function(err) {
              
              var stats = Fs.statSync(tmp_file)
              var size = stats["size"];
              
              if(size < 1000) {
                Fs.unlink(tmp_file, function() {
                  self.log('Image size invalid. ' + size + ' bytes received, removed tmp file ' + tmp_file);
                  dl_img(queue);
                });
              } else {
              
                // Move image from tmp to save dir
                
                Fs.rename(tmp_file, dest_file, function() {
                  dl_img(queue);
                });
              }
              
            }, function() {
              
              dl_img(queue);
            });
          } else {
            
            dl_img(queue);
          }
        };
        
        dl_img(Array.clone(images));
        
      } else {
        
        self.log('Failed to load ' + url);
        
        this.lastModIdMin = 0;
        this.lastModIdMax = 0;
        
        setTimeout(function() {
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