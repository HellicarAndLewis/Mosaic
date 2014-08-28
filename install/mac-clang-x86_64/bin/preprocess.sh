#!/bin/sh
#set -x
d=${PWD}
magickdir=${d}/../imagemagick/
infile=${1}

function log {
    dat=$(date +%Y.%m.%d.%H.%M.%S)
    echo "${dat}: ${1}" >> log/preprocess.log
}

# Make sure the file exists.
if [ ! -f ${infile} ] ; then
    log "Cannot find ${infile}"
    exit
fi

filename=$(basename "$infile")
extension="${filename##*.}"
filename="${filename%.*}"

resized_filepath=${d}/data/input_resized/${filename}.png
blur_filepath=${d}/data/input_blurred/${filename}.png

# Resize 
${magickdir}/convert ${infile} \
    -resize 64x64^ \
    -gravity center \
    -extent 64x64 \
    -colors 256 \
    PNG8:${resized_filepath}

# Blur resized image
${magickdir}/convert ${resized_filepath} \
    -blur 0x5 \
    -colors 256 \
    PNG8:${blur_filepath}



