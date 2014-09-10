/************************************************************
 *
 * ExpressJS Routes
 *
 ***********************************************************/

var Console = require('./Console');
var Express = require('express');
var BodyParser = require('body-parser');
var Fs = require('fs');
var ObjectID = require('mongodb').ObjectID;
var Dot = require('dot');
var CookieSession = require('cookie-session');
var CookieParser = require('cookie-parser');
var MethodOverride = require('method-override');

// Tag subscription router
// --------------------------------------------------------
var TagSubscription = new Class({
  
  Implements: [process.EventEmitter]
  
  // Events
  ,NEW_TAG: 'new_tag'
  ,VERIFIED: 'verified'
  
  // Constructor
  // --------------------------------------------------------
  ,initialize: function(app) {
    
    this.app = app;
    
    var self = this;
    
    // Create express router
    this.router = Express.Router();
    
    // Use bodyparser to parse post data
    this.router.use(BodyParser.urlencoded({extended: false}));
    this.router.use(BodyParser.json());
    
    // Create get route
    this.router.get('/tag/:tagName', function(req, res) {
      
      // Fire verified event
      self.emit(self.VERIFIED);
      
      Console.status('Instagram authenticated');
      
      // Send back challenge code
      res.set('Content-Type', 'text/html');
      res.send(req.query['hub.challenge']);
    });
    
    // Create post route
    this.router.post('/tag/:tagName', function(req, res) {
      
      // Fire new tag event
      self.emit(self.NEW_TAG, req.body);
    });
  
  }
});

module.exports.TagSubscription = TagSubscription;

var UNLOCK_TIME = (60*60*1000);

// Admin router
// --------------------------------------------------------
var Admin = new Class({
  
  Implements: [process.EventEmitter]
  
  // Constructor
  // --------------------------------------------------------
  ,initialize: function(app) {
    
    this.app = app;
    
    var self = this;
    
    // Create express router
    this.router = Express.Router();
    
    // Serve static
    this.router.use('/admin/css', Express.static(__dirname + '/../html/css'));
    this.router.use('/admin/js', Express.static(__dirname + '/../html/js'));
    this.router.use('/admin/images', Express.static(__dirname + '/../html/images'));
    this.router.use('/admin/fonts', Express.static(__dirname + '/../html/fonts'));
    
    // Modules
    this.router.use(MethodOverride());
    this.router.use(CookieParser());
    this.router.use(CookieSession({secret:'$3cr3tp@$$W0rD'}));
    this.router.use(BodyParser.urlencoded({extended: false}));
    
    // Login form
    this.router.get('/login', function(req, res) {
      
      req.session.iaid = '';
      self.app.iaid = '';
      Fs.readFile(__dirname + '/../html/login.html', 'utf8', function(err, text) {
        res.send(text);
      });
    });
    
    // Logout
    this.router.get('/logout', function(req, res) {
      
      req.session.iaid = '';
      self.app.iaid = '';
      
      res.redirect('/login');
    });
    
    var reset_fn = function(req, res, redirect) {
      
      if((req.params.unlockid == '0' || req.params.unlockid == 0) 
         && (req.params.queueid == '0' || req.params.queueid == 0)) {
        
        if(redirect) { res.redirect('/login'); }
        return;
      }
      
      if(req.params.unlockid != '0' || req.params.unlockid != 0) {
        
        var collection = self.app.db.collection('instagram');
        collection.update(
          {_id:ObjectID(req.params.unlockid)}
          ,{$set:{
            locked: false
            ,reviewed: false
            ,approved: false
            ,queue_id: ObjectID()
          }}
          ,{w:1}
          ,function() {
            
            Console.status('image unlocked: ' + req.params.unlockid);
            
            if((req.params.queueid != '0' || req.params.queueid != 0)) {
              
              // Update
              collection.update(
                {_id: ObjectID(req.params.queueid)}
                ,{
                  $set:{
                    queue_id: ObjectID()
                    ,locked: false
                    ,approved: (req.params.approved == 'true') ? true : false
                    ,modified_time: Date.now()
                    ,reviewed: true
                  }
                }
                ,{w:1}
                ,function() {
                  Console.status('image saved: ' + req.params.queueid);
                  if(redirect) { res.redirect('/login'); }
              });
            } else {
               if(redirect) { res.redirect('/login'); }
            }
            
        });
      }
      
    };
    
    this.router.get('/reset/:unlockid/:queueid/:approved', function(req, res) {
      
      reset_fn(req, res, false);
      
    });
    
    // Login form post
    this.router.post('/login', function(req, res) {
      
      // Unlock images older than x ms
      self.unlockImages(function() {
        
        if(req.body.username == self.app.options.admin.username 
           && req.body.password == self.app.options.admin.password) {

          self.app.iaid = ObjectID();
          req.session.iaid = self.app.iaid;

          res.redirect('/admin/tags'); 

        } else {

          res.redirect('/login');
        }
      }, UNLOCK_TIME);
    });
    
    // Redirect after login
    this.router.get('/', this.validate.bind(this), function(req, res) {
      
      res.redirect('/admin/tags');
    });
    
    // Redirect after login
    this.router.get('/admin', this.validate.bind(this), function(req, res) {
      
      res.redirect('/admin/tags');
    });
    
    // Settings route
    this.router.get('/admin/settings', this.validate.bind(this), function(req, res) {
        
      // Return index.html
      Fs.readFile(__dirname + '/../html/settings.html', 'utf8', function(err, text) {

        var collection = self.app.db.collection('settings');
        var result = collection.find({}).limit(1);

        result.toArray(function(err, docs) {

          var tpl = Dot.template(text);
          res.send(tpl({
            auto_approve_users: self.app.options.instagram.auto_approve_users
            ,host: self.app.options.http.host
            ,port: self.app.options.http.port
            ,show_mosaic: (docs.length > 0) ? parseInt(docs[0].show_mosaic) : 1
          }));
        });

      });
    });
    
    // Settings route
    this.router.post('/settings/update', BodyParser.json(), function(req, res) {
      
      if(req.body.show_mosaic) {
        
        var collection = self.app.db.collection('settings');
        collection.update({}, {$set: {'show_mosaic': parseInt(req.body.show_mosaic)}}, {w:1, upsert:true}, function(err) {
         
          res.json({updated:(self.app.iaid) ? true : false});
        });
      } else {
        res.json({updated:(self.app.iaid != '') ? true : false}); 
      }
    });
    
    this.router.get('/settings/get', BodyParser.json(), function(req, res) {
   
      var collection = self.app.db.collection('settings');
      var result = collection.find({}).limit(1);
      result.toArray(function(err, docs) {
        res.json({show_mosaic: (docs.length > 0) ? parseInt(docs[0].show_mosaic) : 1});
      });
    });
    
    // Tags route
    this.router.get('/admin/tags', this.validate.bind(this), function(req, res) {
      
      // Return index.html
      Fs.readFile(__dirname + '/../html/index.html', 'utf8', function(err, text) {

        var tpl = Dot.template(text);
        res.send(tpl({
          msg_type: 'tag'
          ,auto_approve_users: self.app.options.instagram.auto_approve_users
          ,host: self.app.options.http.host
          ,port: self.app.options.http.port
        }));
      });
    });
    
    // Users route
    this.router.get('/admin/users', this.validate.bind(this), function(req, res) {
      
      // Return index.html
      Fs.readFile(__dirname + '/../html/index.html', 'utf8', function(err, text) {

        var tpl = Dot.template(text);
        res.send(tpl({
          msg_type: 'user'
          ,auto_approve_users: self.app.options.instagram.auto_approve_users
          ,host: self.app.options.http.host
          ,port: self.app.options.http.port
        }));
      });

    });
  
  }
  
  // Validate user
  // --------------------------------------------------------
  ,validate: function(req, res, next) {
    
    if((this.app.iaid && (this.app.iaid != '')) && req.session.iaid == this.app.iaid) {
      
      next();
      
    } else {
      
      res.redirect('/login');
    }
  }
  
  // Unlock images older than 30 min
  // --------------------------------------------------------
  ,unlockImages: function(callback, ms) {

      var collection = this.app.db.collection('instagram');
      
      // Find all images that are locked
      var result = collection.find({
 
        locked: true
        ,approved: false
        ,reviewed: false
        ,locked_time: {$lt: Date.now()-ms}
      })
      .sort({locked_time:1});

      result.toArray(function(err, docs) {

        var docs_ids = new Array();

        // Unlock documents
        docs.each(function(doc, i) {
          docs_ids.push(doc._id);
        });

        collection.update(
          {_id:{$in:docs_ids}}
          ,{$set:{
            locked: false
            ,reviewed: false
            ,approved: false
            ,queue_id: ObjectID()
          }}
          ,{w:1, multi:true}
          ,function() {

            Console.status(docs.length + ' images unlocked');
            callback();
        });
      });
    
  }
});

module.exports.Admin = Admin;

// Images router
// --------------------------------------------------------
var Images = new Class({
  
  Implements: [process.EventEmitter]
  
  // Constructor
  // --------------------------------------------------------
  ,initialize: function(app) {
    
    this.app = app;
    
    var self = this;
    
    // Create express router
    this.router = Express.Router();
    
    // Use bodyparser to parse post data
    this.router.use(BodyParser.urlencoded({extended: false}));
    
    // Create get route
    this.router.get('/images/:action/:type/:min_id/:max_id/:limit', function(req, res) {

      // Set type
      var type = ['user', 'tag'];
      if(req.params.type == 'user') {
        type = ['user'];
      } else if(req.params.type == 'tag') {
        type = ['tag'];
      }
 
      // Get queued images
      if(req.params.action == 'queued') {
         
        var collection = self.app.db.collection('instagram');

        var result = collection.find({
          locked: false
          ,approved: false
          ,reviewed: false
          ,msg_type: {$in: type}
        }, {hint:{queue_id:-1}}).sort({queue_id:-1}).limit(parseInt(req.params.limit));

        result.toArray(function(err, docs) {

          var docs_ids = new Array();

          // Lock documents
          docs.each(function(doc, i) {
            docs_ids.push(doc._id);
          });

          collection.update(
            {_id:{$in:docs_ids}}
            ,{
              $set:{
                locked: true
                ,locked_time: Date.now()
              }
            }
            ,{w:1, multi:true}
            ,function() {

              // Output json docs
              res.json(docs);
              res.end();
          });


        });
        
      }
      
      // Get approved images
      if(req.params.action == 'approved') {
        
        var collection = self.app.db.collection('instagram');
        
        if(req.params.min_id != '0' && req.params.max_id != '0') {
  
          var max_result = collection.find({
            locked: false
            ,approved: true
            ,reviewed: true
            ,queue_id: {$gt: ObjectID(req.params.max_id)}
            ,msg_type: {$in: type}
          }, {hint:{queue_id:1}}).sort({queue_id:1}).limit(parseInt(req.params.limit));
          
          // Find images later than the max queue id
          max_result.toArray(function(err, docs) {

            // Check for docs
            if(docs.length > 0) {
              
              // Output json docs
              res.json(docs);
              
            } else {
              
              // Find images earlier than the min queue id
              var min_result = collection.find({
                locked: false
                ,approved: true
                ,reviewed: true
                ,queue_id: {$lt: ObjectID(req.params.min_id)}
              },{hint:{queue_id:-1}}).sort({queue_id:-1}).limit(parseInt(req.params.limit));
              
              min_result.toArray(function(err, docs) {
                
                // Output json docs
                res.json(docs);
                res.end();
              });
              
            }
          });
          
        } else {
          
          // Find next images in queue
          var result = collection.find({
            reviewed: true
            ,approved: true
          },{hint:{queue_id:-1}}).sort({queue_id:-1}).limit(parseInt(req.params.limit));

          result.toArray(function(err, docs) {
            
            if(err) {
              Console.log(err);
              docs = [];
            }
            // Output json docs
            res.json(docs);
            res.end();
          });
        
        }
      }
      
    });
    
    
    // Create post route (update)
    this.router.post('/images/update', function(req, res) {
      
      // Check for id
      if(req.body.id) {

        var collection = self.app.db.collection('instagram');  

        // Update
        collection.update(
          {_id: ObjectID(req.body.id)}
          ,{
            $set:{
              queue_id: ObjectID()
              ,locked: false
              ,approved: (req.body.approved == 'true') ? true : false
              ,modified_time: Date.now()
              ,reviewed: true
            }
          }
          ,{w:1}
          ,function() {
            res.json({updated:(self.app.iaid != '') ? true : false});
            res.end();
        });
      }
    });
  }
  
  // Unlock images older than 30 min
  // --------------------------------------------------------
  ,unlockImages: function(callback, ms) {

      var collection = this.app.db.collection('instagram');
      
      // Find all images that are locked
      var result = collection.find({
 
        locked: true
        ,approved: false
        ,reviewed: false
        ,locked_time: {$lt: Date.now()-ms}
      })
      .sort({locked_time:1});

      result.toArray(function(err, docs) {

        var docs_ids = new Array();

        // Unlock documents
        docs.each(function(doc, i) {
          docs_ids.push(doc._id);
        });

        collection.update(
          {_id:{$in:docs_ids}}
          ,{$set:{
            locked: false
            ,reviewed: false
            ,approved: false
            ,queue_id: ObjectID()
          }}
          ,{w:1, multi:true}
          ,function() {

            Console.status(docs.length + ' images unlocked');
            callback();
        });
      });
    
  }
});

module.exports.Images = Images;