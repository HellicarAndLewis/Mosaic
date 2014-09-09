#!/bin/sh
d=${PWD}
while true 
do
    # must be a full path
    cd /Users/roxlu/Documents/programming/topshop/projects/admin/apps/image_downloader/src
    /usr/local/bin/node server.js -s settings_topshop.json
    sleep 30
done
