/************************************************************
 *
 * Server
 *
 ***********************************************************/
require('mootools');

var Express = require('express');
var Program = require('commander');
var Http = require('http');
var Path = require('path');
var Fs = require('fs');
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
    
    "admin": {
      "username": "",
      "password": ""
    },
    "instagram": {
      "client_id": ""
      ,"client_secret": ""
      ,"hashtags": []
      ,"users": []
      ,"request_delay": 2000
    }
    ,"http": {
      "host": "localhost"
      ,"port": 3333
    }
    ,"mongodb": {
      "host": "localhost"
      ,"port": 27017
      ,"db": "mosaic"
    }
  }
  
  // Constructor
  // --------------------------------------------------------
  ,initialize: function() {
    
    // Process args
    this.args = Program
      .version('0.0.1')
      .option('-s, --settings [path]', 'Start with custom settings file')
      .option('-n, --nopolling', 'Start without media polling')
      .option('-t, --polltags', 'Start with tag media polling')
      .option('-u, --pollusers', 'Start with users media polling')
      .option('-c, --clear', 'Clear all instagram subscriptions')
      .parse(process.argv);
    
    Console.DEBUG = false;
    
    var file = './settings.json';
    
    if(Program.settings) {
      file = Path.normalize(process.cwd() + '/' + Program.settings);
    }
   
    // Check if file exists...
    if(Fs.existsSync(file)) {
    
      // Include project file
      var settings = require(file);
      
      Console.status('Started with settings ' + file);
      
      // Set options
      this.setOptions(settings);
      
      // Connect mongo db
      this.connectDb();
      
    } else {
    
      // No file found
      Console.error('Could not find settings file at ' + file);
    }

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
      
      ,function(err, db) {

        if(err) {
          Console.error('Could not connect to DB');
          process.exit(0);
        }
        
        Console.info('MongoDb connected');
        
        self.db = db;
        
        // Setup server
        self.setupServer();
        
        // Setup Instagram api
        self.setupInstagramApi();
      }
    
    );
  }
  
  // Setup http server and socket.io
  // --------------------------------------------------------
  ,setupServer: function() {
    
    // Create expressjs app
    this.app = Express();
    
    // Setup http server
    this.server = Http.Server(this.app);
    
    // Setup socket io
    //this.socket = new SocketIo(this.server);
    
    // Tag subscription router
    //this.tagSubscriptionRouter = new Routes.TagSubscription(this);
    //this.app.use(this.tagSubscriptionRouter.router);
    
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
    
    var self = this;
    
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
    
    this.getUsers(function() {
      
      // Get recent media
      if(!Program.nopolling) {
        
        if(!Program.polltags && !Program.pollusers) {
          
          // Get recent tag media
          self.options.instagram.hashtags.each(function(tag, i) {
            self.getTagRecentMedia(tag);
          });

          // Get recent user media
          self.users.each(function(user, i) {
            self.getUserRecentMedia(user);
          });
        
        } else if(Program.polltags && !Program.pollusers) {
          
          // Get recent tag media
          self.options.instagram.hashtags.each(function(tag, i) {
            self.getTagRecentMedia(tag);
          });
        
        } else if(!Program.polltags && Program.pollusers) {
          
          // Get recent user media
          self.users.each(function(user, i) {
            self.getUserRecentMedia(user);
          });
        }
        
      }
      
    });

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
  
  // Get users
  // --------------------------------------------------------
  ,getUsers: function(callback) {
    
    var self = this;
    this.users = new Array();

    var get_user = function(queue, cb) {
      
      if(queue.length == 0) {
        cb();
        return;
      }
      
      var username = queue.pop();
      
      self.instagram.getUser(username, function(err, users) {
        
        if(users.length > 0) {
          self.users.push(users[0]);
        } else {
          Console.error('Could not find user ' + username); 
        }
        
        get_user(queue, cb);
        
      });
    };

    get_user(Array.clone(this.options.instagram.users), callback);
    
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
        }, self.options.instagram.request_delay); 
      }
      
      // 503 error
      if(err) {
     
        // Check for 503 status code
        if(err.status_code == 503) {
          Console.error('503 Service Unavailable. No server is available to handle this request.');
        } else {
          Console.error('Get recent media request failed.');
        }
        
        // Retry after x seconds
        retry();
        
        return;
      }
      
      Console.status('Received recent media for tag ' + tag);
      Console.status(remaining + ' remaining calls');
      
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
                  ,msg_type: 'tag'
                  ,queue_id: ObjectID()
                  ,images: media.images
                  ,user: media.user
                  ,filter: media.filter
                  ,tags: media.tags
                  ,link: media.link
                  ,likes: media.likes.count
                  ,location: media.location
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
          
          if(new_medias.length > 0) {
            
            Console.status(new_medias.length + ' images added for tag ' + tag);
            
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
            Console.status('No new images found for tag ' + tag);
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
  
  // Get user recent media
  // --------------------------------------------------------
  ,getUserRecentMedia: function(user) {
    
    var self = this;
   
    // Get recent media for user callback
    var rm_callback = function(err, medias, pagination, remaining, limit) {
      
      // Retry after x seconds
      // if media is not updated or
      // failed to return any images
      var retry = function() {
        
        setTimeout(function() {
          var opt = {};
          
          if(pagination) {
            if(pagination.next_max_id) {
              var opt = {
                next_max_id: pagination.next_max_id
              }; 
            }
          }
          
          self.instagram.getUserRecentMedia(user.id, opt, rm_callback);
        }, self.options.instagram.request_delay); 
      }
      
      // 503 error
      if(err) {
     
        // Check for 503 status code
        if(err.status_code == 503) {
          Console.error('503 Service Unavailable. No server is available to handle this request. Possible cause is too ouch calls per hour or Instagram is having a hard time.');
        } else {
          Console.error('Get recent media request failed for ' + user.username + '. Make sure the profile is public!');
        }
        
        // Retry after x seconds
        retry();
        
        return;
      }
      
      Console.status('Received recent media for user ' + user.username);
      Console.status(remaining + ' remaining calls');
      
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
          
          // Check for image type, existing media id
          if(media.type = 'image' && media.id) {
            
            // Check if media has one of the set hashtags
            var has_hashtag = false;
            
            media.tags.each(function(tag, i) {
              if(self.options.instagram.hashtags.indexOf(tag) > -1) {
                 has_hashtag = true;
              }
            });
            
            // If media has a corresponding hashtag
            if(has_hashtag) {
              
              // Check if media already exists
              var exists = collection.find({media_id: media.id}, {_id: 1}).limit(1);
              exists.count(function(err, count) {

                // If media doesn't exist
                if(count == 0) {

                  // Create media object
                  new_medias.push({

                    media_id: media.id
                    ,msg_type: 'user'
                    ,queue_id: ObjectID()
                    ,images: media.images
                    ,user: media.user
                    ,filter: media.filter
                    ,tags: media.tags
                    ,link: media.link
                    ,likes: media.likes.count
                    ,location: media.location
                    ,created_time: parseInt(media.created_time)
                    ,modified_time: Date.now()
                    ,locked_time: Date.now()
                    ,locked: false
                    ,approved: self.options.instagram.auto_approve_users
                    ,reviewed: self.options.instagram.auto_approve_users
                  });
                }

                next_media(list, callback);

              });
              
            } else {
              
              // Next if tag is not in list
              next_media(list, callback);
            }
          }
        };
        
        // Check all media and add to db
        // if media does not exist
        next_media(Array.clone(medias), function() {
    
          if(new_medias.length > 0) {
            
            Console.status(new_medias.length + ' images added for user ' + user.username);
            
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
            Console.status('No new images found for user ' + user.username);
            retry(); 
          }
        });
      } else {
        retry(); 
      }
    }
    
    // Get recent media for tag
    this.instagram.getUserRecentMedia(user.id, {}, rm_callback);
  }
  
});

// Start server
Server = new Server();