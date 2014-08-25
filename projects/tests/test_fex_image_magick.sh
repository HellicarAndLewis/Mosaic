#!/bin/sh

d=${PWD}
magickdir=${d}/../../install/mac-clang-x86_64/imagemagick
installdir=${d}/../../install/mac-clang-x86_64/bin

if [ ! -d ${installdir}/test ] ; then 
    mkdir ${installdir}/test
fi

cd ${magickdir}

# Resize to 64x64
./convert ${d}/test_fex_imagemagick.jpg \
    -resize 64x64^ \
    -gravity center \
    -extent 64x64 \
    ${installdir}/test/test_fex_imagemagix_resized_64x64.jpg

# Resize to 64x64 with an image which is not a square
./convert ${d}/test_fex_imagemagick_wrong_size.jpg \
    -resize 64x64^ \
    -gravity center \
    -extent 64x64 \
    ${installdir}/test/test_fex_imagemagix_resized_wrong_size_64x64.jpg

# Blur
./convert ${d}/test_fex_imagemagick.jpg \
    -blur 0x20 \
    ${installdir}/test/test_fex_imagemagix_blurred.jpg

