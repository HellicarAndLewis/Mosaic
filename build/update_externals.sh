#!/bin/sh
d=${PWD}
cd ${d}/../extern/tinylib 
git remote update
git merge origin/master

cd ${d}/../extern/video_capture/shared/tinylib/
git remote update
git merge origin/master 
