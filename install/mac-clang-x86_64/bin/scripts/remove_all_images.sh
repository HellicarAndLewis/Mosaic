#!/bin/sh
d=${PWD}

if [ ! -d ${d}/../data/raw_left ] ; then
    echo "Cannot find ${d}/../data/raw_left"
    exit
fi

cd ${d}/../data/raw_left
rm *.jpg
rm *.png

cd ${d}/../data/raw_right
rm *.jpg
rm *.png

cd ${d}/../data/raw_mosaic
rm *.jpg
rm *.png

cd ${d}/../data/input_grid_left
rm *.jpg
rm *.png

cd ${d}/../data/input_grid_right
rm *.jpg
rm *.png

cd ${d}/../data/input_mosaic
rm *.jpg
rm *.png

cd ${d}/../data/polaroids
rm *.jpg
rm *.png





