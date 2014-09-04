#!/bin/sh
#set -x
d=${PWD}
id=${d}/../install/mac-clang-x86_64
dd=${id}/bin/data



if [ ! -f ${d}/mosaic_test_data.tar.gz ] ; then
    echo "Downloading test data."
    curl -o mosaic_test_data.tar.gz http://upload.roxlu.com/mosaic_test_data.tar.gz
fi

if [ ! -f ${d}/mosaic_test_data.tar.gz ] ; then
    echo "Cannot find the mosaic_test_data file."
    exit
fi

if [ ! -d ${d}/tmp ] ; then
    mkdir ${d}/tmp
fi


if [ ! -f ${d}/tmp/mosaic_test_data.tar.gz ] ; then
    cp -v ${d}/mosaic_test_data.tar.gz ${d}/tmp
fi

cd ${d}/tmp
if [ ! -d bin ] ; then
    tar -zxvf mosaic_test_data.tar.gz
fi

if [ -f ${dd}/descriptors.txt ] ; then
    echo ""
    echo "It seems that you already have some descriptors. Remove descriptors.txt if you want to use the test data"
    echo ""
    exit
fi

td=${d}/tmp/bin/data
cp -vr ${td}/descriptors.txt ${dd}/
cp -vr ${td}/input_grid_left ${dd}/
cp -vr ${td}/input_grid_right ${dd}/
cp -vr ${td}/input_mosaic ${dd}/
cp -vr ${td}/settings ${dd}/
cp -vr ${td}/video ${dd}/


rm -r ${d}/tmp
