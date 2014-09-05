#!/bin/sh
#set -x
d=${PWD}
magickdir=${d}/../imagemagick/
infile=${1}
tile_width=${2}
tile_height=${3}
output_dir=${4}
raw_mosaic_dir=${5}

function log {
    dat=$(date +%Y.%m.%d.%H.%M.%S)
    echo "${dat}: ${1}" >> data/log/preprocess_left.log
}

# Make sure the file exists.
if [ ! -f ${infile} ] ; then
    log "Cannot find ${infile}"
    exit
fi

filename=$(basename "$infile")
extension="${filename##*.}"
filename="${filename%.*}"

grid_filepath=${output_dir}/${filename}.png

# Resize 
${magickdir}/convert ${infile} \
    -resize ${tile_width}x${tile_height}^ \
    -gravity center \
    -extent ${tile_width}x${tile_height} \
    -colors 256 \
    PNG8:tmp.png

mv tmp.png ${grid_filepath}

mv ${infile} ${raw_mosaic_dir}/${filename}.${extension}
