#!/bin/sh
#set -x
d=${PWD}
magickdir=${d}/../imagemagick/
infile=${1}
tile_width=${2}
tile_height=${3}
output_dir=${4}

function log {
    dat=$(date +%Y.%m.%d.%H.%M.%S)
    echo "${dat}: ${1}" >> data/log/preprocess.log
}

# Make sure the file exists.
if [ ! -f ${infile} ] ; then
    log "Cannot find ${infile}"
    exit
fi

filename=$(basename "$infile")
extension="${filename##*.}"
filename="${filename%.*}"

#resized_filepath=${d}/data/input_resized/${filename}.png
resized_filepath=${output_dir}/${filename}.png

# Resize to tile format
${magickdir}/convert ${infile} \
    -resize ${tile_width}x${tile_height}^ \
    -gravity center \
    -extent ${tile_width}x${tile_height} \
    -colors 256 \
    PNG24:${resized_filepath}

rm ${infile}

# ${magickdir}/convert ${infile} \
#     -resize 64x64^ \
#     -gravity center \
#     -extent 64x64 \
#     -colors 256 \
#     PNG8:${resized_filepath}


# Blur resized image (not used atm).
# blur_filepath=${d}/data/input_blurred/${filename}.png
# ${magickdir}/convert ${resized_filepath} \
#     -blur 0x5 \
#     -colors 256 \
#     PNG8:${blur_filepath}



