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

var Console = require('./Console');
var Instagram = require('./Instagram');
var Routes = require('./Routes');

var Server = new Class({
  
  Implements: [Options]
  
  // Options
  ,options: {
    
    client_id: '8357c3f32006446f9fd97d907e1a2954'
    ,client_secret: 'd20bc262654f47c0a90ecda76af22120'
    ,host: '77.248.89.153'
    ,port: 3333 
  }
  
  // Constructor
  // --------------------------------------------------------
  ,initialize: function() {
    
    var self = this;
    
    // Process args
    this.args = Program
      .version('0.0.1')
      .option('-c, --clear', 'Clear all subscriptions')
      .parse(process.argv);
    
    // Create expressjs app
    this.app = Express();
    
    // Setup socket io
    this.server = Http.Server(this.app);
    this.socket = new SocketIo(this.server);
    
    // Use tag route
    this.tagRouter = new Routes.Tag();
    this.app.use(this.tagRouter.router);
    
    // Use admin route
    this.adminRouter = new Routes.Admin();
    this.app.use(this.adminRouter.router);
    
    // Start listening
    this.server.listen(this.options.port);
  
    // Create Instagram api
    this.instagram = new Instagram({
      
      client_id: this.options.client_id
      ,client_secret: this.options.client_secret
    });
    
    // Clear subscriptions
    if(Program.clear) {
      
      this.clearSubscriptions();
      return;
    }

    Console.info('Server started');
    
    // Get recent medua
    this.getTagRecentMedia('test');
    
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
        
    // Get recent tag media
    var rm_callback = function(err, medias, pagination, remaining, limit) {
 
      Console.status('Received recent media for tag ' + tag);
      Console.status('  ' + remaining + ' remaining calls');
      Console.status('  ' + medias.length + ' media found');
    
      var retry = function() {
        
        setTimeout(function() {

          self.instagram.getTagRecentMedia(tag, {}, rm_callback);
        }, 5000); 
      }
      
      if(medias) {
   
        if(medias[0].id != self.lastMediaId) {
          
          self.socket.emit('medias', medias);
          medias[0].id = self.lastMediaId;
        }
        
        if(pagination) {
          if(pagination.next) {
            pagination.next(rm_callback);
          } else {
            retry();
          }
        } else {
          retry();
        }  
      } else {
        retry(); 
      }
    }
    
    this.instagram.getTagRecentMedia(tag, {}, rm_callback);
  }
  
});

// Start server
Server = new Server();