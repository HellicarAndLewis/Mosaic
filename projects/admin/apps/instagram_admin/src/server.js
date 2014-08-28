/************************************************************
 *
 * Server
 *
 ***********************************************************/
require('mootools');

var Express = require('express');
var Program = require('commander');
var Http = require('http');
var SocketIo = require('socket.io');
var MongoClient = require('mongodb').MongoClient;
var ObjectID = require('mongodb').ObjectID;

var Console = require('./Console');
var Instagram = require('./Instagram');
var Routes = require('./Routes');

var Server = new Class({
  
  Implements: [Options]
  
  // Options
  ,options: {
    
    instagram: {
      client_id: '8357c3f32006446f9fd97d907e1a2954'
      ,client_secret: 'd20bc262654f47c0a90ecda76af22120'
    }
    ,http: {
      host: '77.248.89.153'
      ,port: 3333
    }
    ,mongodb: {
      host: 'localhost'
      ,port: 27017
      ,db: 'mosaic'
    }
  }
  
  // Constructor
  // --------------------------------------------------------
  ,initialize: function() {
    
    var self = this;
    
    // Process args
    this.args = Program
      .version('0.0.1')
      .option('-c, --clear', 'Clear all instagram subscriptions')
      .option('-n, --nopoll', 'Start without media polling')
      .parse(process.argv);
    
    this.connectDb();
  }
  
  // Connect to mongodb
  // --------------------------------------------------------
  ,connectDb: function() {
    
    var self = this;
    
    MongoClient.connect(
      
      'mongodb://' 
      + this.options.mongodb.host 
      + ':' + this.options.mongodb.port 
      + '/' + this.options.mongodb.db
      
      , function(err, db) {

        if(err) throw err;
        
        Console.info('MongoDb connected');
        
        self.db = db;
        
        self.setupServer();
        self.setupInstagramApi();
      }
    
    );
  }
  
  // Setup http server and socket.io
  // --------------------------------------------------------
  ,setupServer: function() {
    
    // Create expressjs app
    this.app = Express();
    
    // Setup socket io
    this.server = Http.Server(this.app);
    this.socket = new SocketIo(this.server);
    
    // Tag subscription router
    this.tagSubscriptionRouter = new Routes.TagSubscription(this);
    this.app.use(this.tagSubscriptionRouter.router);
    
    // Admin router
    this.adminRouter = new Routes.Admin(this);
    this.app.use(this.adminRouter.router);
    
    // Images router
    this.imagesRouter = new Routes.Images(this);
    this.app.use(this.imagesRouter.router);
    
    // Start http server
    this.server.listen(this.options.http.port);
    
    Console.info('Server started');
    
  }
  
  // Setup Instagram API
  // --------------------------------------------------------
  ,setupInstagramApi: function() {
    
    // Create Instagram api
    this.instagram = new Instagram({
      
      client_id: this.options.instagram.client_id
      ,client_secret: this.options.instagram.client_secret
    });
    
    // Clear subscriptions
    if(Program.clear) {
      
      this.clearSubscriptions();
      return;
    }

    // Get recent media
    if(!Program.nopoll) {
      this.getTagRecentMedia('beautiful');
    }
    
  }
  
  // Clear Instagram subscriptions
  // --------------------------------------------------------
  ,clearSubscriptions: function() {
    
    Console.status('Clear request send..please wait');

    // Clear existing subscriptions
    this.instagram.clearSubscriptions(function() {
      
      Console.info('All done!');
      process.exit(0);
    });

  }
  
  // Get recent media
  // --------------------------------------------------------
  ,getTagRecentMedia: function(tag) {
    
    var self = this;
        
    // Get recent media for tag callback
    var rm_callback = function(err, medias, pagination, remaining, limit) {
      
      // Retry after x seconds
      // if media is not updated or
      // failed to return any images
      var retry = function() {
        
        setTimeout(function() {
          var opt = {};
          
          if(pagination) {
            if(pagination.min_tag_id) {
              var opt = {
                min_tag_id: pagination.min_tag_id
              }; 
            }
          }
          
          self.instagram.getTagRecentMedia(tag, opt, rm_callback);
        }, 2000); 
      }
      
      if(err) {
        
        if(err.status_code == 503) {
          Console.error('503 Service Unavailable. No server is available to handle this request.');
        } else {
          Console.error('Get recent media request failed.');
        }
        retry();
        return;
      }
      
      //Console.status('Received recent media for tag ' + tag);
      Console.status('  ' + remaining + ' remaining calls');
      //Console.status('  ' + medias.length + ' media found');
      
      // Empty check
      if(medias) {
        
        var collection = self.db.collection('instagram');
  
        // Fill new array with all media
        var new_medias = new Array();
        
        // Iterator
        var next_media = function(list, callback) {
          
          if(list.length == 0) {
            callback();
            return;
          }
          
          var media = list.pop();
          
          // Check for image type and existing id
          if(media.type = 'image' && media.id) {
            
            // Check if media already exists
            var exists = collection.find({media_id: media.id}, {_id: 1}).limit(1);
            exists.count(function(err, count) {
             
              // If media doesn't exist
              if(count==0) {
                
                // Create media object
                new_medias.push({

                  media_id: media.id
                  ,images: media.images
                  ,user: media.user
                  ,filter: media.filter
                  ,tags: media.tags
                  ,link: media.link
                  ,created_time: parseInt(media.created_time)
                  ,modified_time: Date.now()
                  ,locked_time: Date.now()
                  ,locked: false
                  ,approved: false
                  ,reviewed: false
                });
              }
              
              next_media(list, callback);
              
            });
          }
        };
        
        // Check all media and add to db
        // if media does not exist
        next_media(Array.clone(medias), function() {
          
          
          Console.status(new_medias.length + ' media added');
          
          if(new_medias.length > 0) {
            
            collection.insert(new_medias, {w:1}, function(err, result) {

              if(err) throw err;

              // Call next page if pagination
              // is available
              if(pagination) { 
                if(pagination.next) { 
                  pagination.next(rm_callback); 
                } else { 
                  retry(); 
                }
              } else {
                retry();
              }  
            });
          } else {
            retry(); 
          }
        });

      } else {
        retry(); 
      }
    }
    
    // Get recent media for tag
    this.instagram.getTagRecentMedia(tag, {}, rm_callback);
  }
  
});

// Start server
Server = new Server();