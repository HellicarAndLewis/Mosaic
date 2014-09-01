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
    
    // Create admin route
    this.router.get('/admin', function(req, res) {
      
      // Unlock images older than x ms
      self.unlockImages(function() {
        
        // Return index.html
        Fs.readFile(__dirname + '/../html/index.html', 'utf8', function(err, text) {
          res.send(text);
        });
      }, (30*60*1000));
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
      .sort({lock_time:1});

      result.toArray(function(err, docs) {

        var docs_ids = new Array();

        // Unlock documents
        docs.each(function(doc, i) {
          docs_ids.push(doc._id);
        });

        collection.update(
          {_id:{$in:docs_ids}}
          ,{$set:{locked: false, reviewed: false}}
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
    this.router.get('/images/:action/:limit/:min_id/:max_id', function(req, res) {
      
      // Get queued images
      if(req.params.action == 'queued') {
       
        var collection = self.app.db.collection('instagram');
        
        var result = collection.find({
          locked: false
          ,approved: false
          ,reviewed: false
        }).sort({queue_id:-1}).limit(parseInt(req.params.limit));
        
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
          }).sort({_id:1}).limit(parseInt(req.params.limit));
          
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
              }).sort({queue_id:-1}).limit(parseInt(req.params.limit));
              
              min_result.toArray(function(err, docs) {
                
                // Output json docs
                res.json(docs);
              });
              
            }
          });
          
        } else {
          
          // Find next images in queue
          var result = collection.find({
            locked: false
            ,approved: true
            ,reviewed: true
          }).sort({queue_id:-1}).limit(parseInt(req.params.limit));

          result.toArray(function(err, docs) {

            // Output json docs
            res.json(docs);
          });
        }
      }
      
    });
    
    
    // Create post route (update)
    this.router.post('/images/update', function(req, res) {
 
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
            res.json({updated:true});
        });
        
      }
      
    });
  
  }
});

module.exports.Images = Images;