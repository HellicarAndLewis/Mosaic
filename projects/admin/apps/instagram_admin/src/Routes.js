/************************************************************
 *
 * ExpressJS Routes
 *
 ***********************************************************/

var Console = require('./Console');
var Express = require('express');
var BodyParser = require('body-parser');
var Fs = require('fs');

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
    
    this.router = Express.Router();

    this.router.use(BodyParser.urlencoded({extended: false}));
    this.router.use(BodyParser.json());

    this.router.get('/tag/:tagName', function(req, res) {
    
      self.emit(self.VERIFIED);
      Console.status('Instagram authenticated');
      
      res.set('Content-Type', 'text/html');
      res.send(req.query['hub.challenge']);
    });
    
    this.router.post('/tag/:tagName', function(req, res) {
      
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
    
    this.router = Express.Router();
    
    this.router.use('/admin/css', Express.static(__dirname + '/../html/css'));
    this.router.use('/admin/js', Express.static(__dirname + '/../html/js'));
    
    this.router.get('/admin', function(req, res) {
      
      Fs.readFile(__dirname + '/../html/index.html', 'utf8', function(err, text) {
        res.send(text);
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
    
    this.router = Express.Router();
    this.router.get('/images/:action/:limit', function(req, res) {
      
      // Get queued images
      if(req.params.action == 'queued') {
       
        var collection = self.app.db.collection('instagram');
        
        var result = collection.find({
          locked: false
          ,approved: false
          ,reviewed: false
        })
        .sort({_id:-1})
        .limit(parseInt(req.params.limit));
        
        result.toArray(function(err, docs) {
          
          var docs_ids = new Array();
          
          // Lock documents
          docs.each(function(doc, i) {
            docs_ids.push(doc._id);
          });
          
          var ntime = Date.now().toString();
          
          collection.update(
            {_id:{$in:docs_ids}}
            ,{$set:{'locked':true, locked_time:ntime}}
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
        
        var result = collection.find({
          locked: false
          ,approved: true
          ,reviewed: true
        })
        .sort({_id:-1})
        .limit(parseInt(req.params.limit));
        
        result.toArray(function(err, docs) {
          
          // Output json docs
          res.json(docs);
        });
      }
    });
  
  }
});

module.exports.Images = Images;