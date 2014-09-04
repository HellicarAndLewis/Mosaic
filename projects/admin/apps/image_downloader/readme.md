Instagram Image Downloader
=========
 

Version
----

0.0.1


Installation
--------------

- Copy *./projects/apps/admin/image_downloader/settings.json* to *settings_your_name.json* in the same folder.
- Change to values in settings_your_name.json

Then enter the following commands in terminal:

```sh
$ cd ./project/admin/apps/image_downloader
$ sudo npm install
```

Now the admin is ready to start. Make sure you have the Instagram Admin running.

```sh
$ cd ./src
$ node server.js -s settings_your_name.json
```

To view all program options use
```sh
$ cd ./src
$ node server.js --help
```