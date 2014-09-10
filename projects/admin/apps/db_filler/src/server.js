var MongoClient = require('mongodb').MongoClient;
var ObjectID = require('mongodb').ObjectID;

var doc = {
  "_id" : null,
  "media_id" : null,
  "msg_type" : "tag",
  "queue_id" : null,
  "images" : {
    "low_resolution" : {
      "url" : "http://scontent-b.cdninstagram.com/hphotos-xpa1/t51.2885-15/927449_1473627669566987_728513574_a.jpg",
      "width" : 306,
      "height" : 306
    },
    "thumbnail" : {
      "url" : "http://scontent-b.cdninstagram.com/hphotos-xpa1/t51.2885-15/927449_1473627669566987_728513574_s.jpg",
      "width" : 150,
      "height" : 150
    },
    "standard_resolution" : {
      "url" : "http://scontent-b.cdninstagram.com/hphotos-xpa1/t51.2885-15/927449_1473627669566987_728513574_n.jpg",
      "width" : 640,
      "height" : 640
    }
  },
  "user" : {
    "username" : "juaneaa",
    "website" : "",
    "profile_picture" : "http://photos-h.ak.instagram.com/hphotos-ak-xaf1/10358382_1422962921304487_1511912314_a.jpg",
    "full_name" : "Juan Gil",
    "bio" : "",
    "id" : "1095436105"
  },
  "filter" : "Normal",
  "tags" : [
    "beautiful",
    "cute",
    "fashion",
    "love",
    "summer",
    "food",
    "instalike",
    "tbt",
    "igers",
    "follow",
    "instadaily",
    "instamood",
    "friends",
    "girl",
    "me",
    "swag",
    "like4like",
    "tflers",
    "followme",
    "instagood",
    "tagsforlikes",
    "amazing",
    "bestoftheday",
    "fun",
    "smile",
    "photooftheday",
    "picoftheday",
    "happy"
  ],
  "link" : "http://instagram.com/p/swvVLkkUgE/",
  "likes" : 44,
  "location" : null,
  "created_time" : 1410344730,
  "modified_time" : 1410347985896,
  "locked_time" : 1410347985896,
  "locked" : false,
  "approved" : false,
  "reviewed" : false
}

MongoClient.connect('mongodb://mosaic:bsEn%283DyD.{uV@localhost:27017/mosaic' ,function(err, db) {

  if(err) {
    console.error('Could not connect to DB');
    process.exit(0);
  }

  console.info('MongoDb connected');

  var collection = db.collection('instagram');
  
  var num_inserts = 0;
  
  var insert = function() {
    
    if(num_inserts == 300000) {
      console.log('all done');
      return;
    }
    
    var new_doc = JSON.parse(JSON.stringify(doc));
    new_doc._id = ObjectID();
    new_doc.queue_id = ObjectID();
    new_doc.media_id = ObjectID();
    
    collection.insert(new_doc, function(err) {
      if(err) {
        console.error(err);
      }
      
      num_inserts++;
      if(num_inserts%1000==0) {
        console.log('#' + num_inserts);
      }
      insert();
    });
  };
  insert();
});