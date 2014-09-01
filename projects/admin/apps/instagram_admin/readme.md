Mosaic Instagram Admin
=========
 

Version
----

0.0.1


Installation
--------------

- Download and install mongodb: http://www.mongodb.org/downloads
- Download and install nodejs: http://www.nodejs.org/download/
- Copy *./projects/apps/admin/instagram_admin/settings.json* to *settings_your_name.json* in the same folder.
- Change to values in settings_your_name.json

Then enter the following commands in terminal:

```sh
$ cd ./project/admin/apps/instagram_admin
$ sudo npm install
```

Now the admin is ready to start

```sh
$ cd ./src
$ node server.js -s settings_your_name.json
```

Browse to http://your_ip:your_port or with default values go to http://localhost:3333

To view all program options use
```sh
$ cd ./src
$ node server.js --help
```

Users admin
----

- Set *auto_approve_users* in settings_your_name.json to *true*
- Login using http://your_ip:your_port or with default values go to http://localhost:3333
- Navigate to /admin/users


Other
----

**Approve all images**

```sh
$ mongo mosaic
> db.instagram.update({},{$set:{reviewed:true, approved:true, locked:false, modified_time:Date.now()}}, false, true)
```

**Reset all images**

```sh
$ mongo mosaic
> db.instagram.update({},{$set:{reviewed:false, approved:false, locked:false, modified_time:Date.now()}}, false, true)
```

Todo
=========
- Add menu for tags / users view
- add ip whitelist for requests
- add touch control