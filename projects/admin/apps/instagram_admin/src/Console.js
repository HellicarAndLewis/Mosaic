/************************************************************
 *
 * Console color wrapper
 *
 ***********************************************************/

var clc = require('cli-color');

function Console() {

  this.clc_error = clc.whiteBright.bgRedBright.bold;
  this.clc_warn = clc.yellow;
  this.clc_notice = clc.blue.bgWhiteBright.bold;
  this.clc_info = clc.red.bgWhiteBright.bold;
  this.clc_status = clc.yellowBright;
  this.clc_start = clc.whiteBright.bgBlackBright.bold;
  this.clc_end = clc.cyanBright;
}

Console.prototype = {
  
  //--------------------------------------------------------------
  addSpace: function(v) {
  
    return ' ' + v + ' ';
  }
  
  //--------------------------------------------------------------
  ,clear: function() {
  
    console.log(clc.reset);
  }
  
  //--------------------------------------------------------------
  ,log: function(v) {
  
    console.log(v);
  }
  
  //--------------------------------------------------------------
  ,error: function(v) {
    
    var d = new Date();
    var t = d.getHours() + ':' + d.getMinutes() + ':' + d.getSeconds() + ' > ';
    console.error(this.clc_error(this.addSpace(t+v)));
  }
  
  //--------------------------------------------------------------
  ,warn: function(v) {
    
    console.warn(this.clc_warn(v));
  }
  
  //--------------------------------------------------------------
  ,notice: function(v) {
    
    console.info(this.clc_notice(this.addSpace(v)));
  }
  
  //--------------------------------------------------------------
  ,status: function(v) {
    
    var d = new Date();
    var t = d.getHours() + ':' + d.getMinutes() + ':' + d.getSeconds() + ' > ';
    console.info(this.clc_status(t+v));
  }
  
  //--------------------------------------------------------------
  ,start: function(v) {
    
    console.info(this.clc_start(this.addSpace(v)));
  }
  
  //--------------------------------------------------------------
  ,end: function(v) {
    
    console.info(this.clc_end(v));
  }
  
  //--------------------------------------------------------------
  ,empty: function() {
    
    console.log('');
  }
  
  //--------------------------------------------------------------
  ,info: function(v) {
    
    var d = new Date();
    var t = d.getHours() + ':' + d.getMinutes() + ':' + d.getSeconds() + ' > ';
    console.info(this.clc_info(this.addSpace(t+v)));
  }
};

module.exports = new Console();