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
  
  DEBUG: true
  
  //--------------------------------------------------------------
  ,addSpace: function(v) {
  
    return ' ' + v + ' ';
  }
  
  //--------------------------------------------------------------
  ,clear: function() {
    
    if(!this.DEBUG) return;
    console.log(clc.reset);
  }
  
  //--------------------------------------------------------------
  ,log: function(v) {
    if(!this.DEBUG) return;
    console.log(v);
  }
  
  //--------------------------------------------------------------
  ,error: function(v) {
    if(!this.DEBUG) return;
    var d = new Date();
    var t = d.getHours() + ':' + d.getMinutes() + ':' + d.getSeconds() + ' > ';
    console.error(this.clc_error(this.addSpace(t+v)));
  }
  
  //--------------------------------------------------------------
  ,warn: function(v) {
    if(!this.DEBUG) return;
    console.warn(this.clc_warn(v));
  }
  
  //--------------------------------------------------------------
  ,notice: function(v) {
    if(!this.DEBUG) return;
    console.info(this.clc_notice(this.addSpace(v)));
  }
  
  //--------------------------------------------------------------
  ,status: function(v) {
    if(!this.DEBUG) return;
    var d = new Date();
    var t = d.getHours() + ':' + d.getMinutes() + ':' + d.getSeconds() + ' > ';
    console.info(this.clc_status(t+v));
  }
  
  //--------------------------------------------------------------
  ,start: function(v) {
    if(!this.DEBUG) return;
    console.info(this.clc_start(this.addSpace(v)));
  }
  
  //--------------------------------------------------------------
  ,end: function(v) {
    if(!this.DEBUG) return;
    console.info(this.clc_end(v));
  }
  
  //--------------------------------------------------------------
  ,empty: function() {
    if(!this.DEBUG) return;
    console.log('');
  }
  
  //--------------------------------------------------------------
  ,info: function(v) {
    if(!this.DEBUG) return;
    var d = new Date();
    var t = d.getHours() + ':' + d.getMinutes() + ':' + d.getSeconds() + ' > ';
    console.info(this.clc_info(this.addSpace(t+v)));
  }
};

module.exports = new Console();