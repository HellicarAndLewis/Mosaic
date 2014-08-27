/************************************************************
 *
 * ExpressJS Routes
 *
 ***********************************************************/

var Console = require('./Console');
var Express = require('express');
var BodyParser = require('body-parser');
var Fs = require('fs');

// Tag route
// --------------------------------------------------------
var Tag = new Class({
  
  Implements: [process.EventEmitter]
  
  // Events
  ,NEW_TAG: 'new_tag'
  ,VERIFIED: 'verified'
  
  // Constructor
  // --------------------------------------------------------
  ,initialize: function() {
    
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

module.exports.Tag = Tag;


// Admin route
// --------------------------------------------------------
var Admin = new Class({
  
  Implements: [process.EventEmitter]
  
  // Constructor
  // --------------------------------------------------------
  ,initialize: function() {
    
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