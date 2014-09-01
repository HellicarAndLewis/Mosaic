/************************************************************
 *
 * Image downloader
 *
 ***********************************************************/

require('mootools');

var Fs = require('fs');
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
      
      console.log('Started with settings ' + file);
      
      self.settings.image_tmp_path = Path.normalize(process.env['HOME'] + '/' + self.settings.image_tmp_path);
      self.settings.image_save_path = Path.normalize(process.env['HOME'] + '/' + self.settings.image_save_path);
      
      self.start();
      
    } else {
    
      // No file found
      console.log('Could not find settings file at ' + file);
    }

    
  }
  
  // 
  // --------------------------------------------------------
  ,start: function() {
    
    var self = this;
    var url = this.settings.queue_url + '/all/' +  this.lastModIdMin + '/' + this.lastModIdMax + '/' + this.settings.limit + '/';
    
    console.log('Get new results - ' + url);

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
          var dest_file = self.settings.image_save_path + img._id + '.jpg';
          
          // Check if tmp dir exists
          if(!Fs.existsSync(self.settings.image_tmp_path)) {
            
            console.log('Creating tmp dir ' + self.settings.image_tmp_path);
            Fs.mkdirSync(self.settings.image_tmp_path);
          }
          
          // Check if save dir exists
          if(!Fs.existsSync(self.settings.image_save_path)) {
            
            console.log('Creating save dir ' + self.settings.image_save_path);
            Fs.mkdirSync(self.settings.image_save_path);
          }
          
          var file_exists = Fs.existsSync(dest_file);
          if(!file_exists) {
            
            // Download image
            self.download(img.images[self.settings.image_size].url, tmp_file, function(err) {
              
              var stats = Fs.statSync(tmp_file)
              var size = stats["size"];
              
              if(size < 1000) {
                Fs.unlink(tmp_file, function() {
                  console.log('Image size invalid. ' + size + ' bytes received, removed tmp file ' + tmp_file);
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
        
        console.log('Failed to load ' + url);
        
        this.lastModIdMin = 0;
        this.lastModIdMax = 0;
        
        setTimeout(function() {
          self.start();
        }, self.settings.request_delay);
        return;
      }
    });
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