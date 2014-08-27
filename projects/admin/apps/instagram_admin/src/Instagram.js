/************************************************************
 *
 * Instagram API
 *
 ***********************************************************/

require('mootools');

var Console = require('./Console');


var Instagram = new Class({
  
  Implements: [process.EventEmitter, Options]
  
  // Options
  ,options: {
    
    access_token: ''
    ,client_id: ''
    ,client_secret: ''
  }
  
  // Constructor
  // --------------------------------------------------------
  ,initialize: function(options) {
    
    var self = this;
    
    this.setOptions(options);

    this.api = require('instagram-node').instagram();
    this.api.use(this.options);

  }
  
  // Create a tag subscription
  // --------------------------------------------------------
  ,subscribeTag: function(tag, callbackUrl) {
    
    Console.status('Subscribe to tag: ' + tag);
    var self = this;
    
    this.api.add_tag_subscription(
      tag
      ,callbackUrl
      ,function(err, result, remaining, limit) {
        
        if(err) {
          Console.error('Failed to subscribe to ' + tag +' with id ' + result.id);
          Console.error(err); 
        }
        Console.status('Subscribed to ' + tag + ' with id ' + result.id + ', ' + remaining + ' remaining requests');
      }
    );
  }
  
  // Get info about tag
  // --------------------------------------------------------
  ,getTag: function(tag, options, callback) {
    
    this.api.tag(tag, options, callback);
  }
  
  // Create a tag subscription
  // --------------------------------------------------------
  ,getTagRecentMedia: function(tag, options, callback) {
    
    this.api.tag_media_recent(tag, options, callback);
  }
  
  // List subscriptions
  // --------------------------------------------------------
  ,listSubscriptions: function(callback) {
    
    this.api.subscriptions(function(err, subscriptions, remaining, limit) {
     
      var has_subscriptions = false;
      if(subscriptions) {
        has_subscriptions = (subscriptions.length > 0);
      }
      callback(has_subscriptions, subscriptions);
    });
  }
  
  // Clear all existing subscriptions
  // --------------------------------------------------------
  ,clearSubscriptions: function(callback) {
    
    this.api.del_subscription({all: true}, callback);
  }
  
});

module.exports = Instagram;