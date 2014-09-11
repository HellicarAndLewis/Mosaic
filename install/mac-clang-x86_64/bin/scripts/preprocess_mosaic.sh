#!/bin/sh
#set -x

# Check mosaic dir
d=${MOSAIC_BINDIR}
if [ -z "${d}" ] ; then
    if [ -f ${PWD}/AppMosaic ] ; then
        d=${PWD}
    else
        d=${PWD}/../
    fi
fi

bindir=${d}  # see /etc/launchd.conf and http://www.dowdandassociates.com/blog/content/howto-set-an-environment-variable-in-mac-os-x-slash-etc-slash-launchd-dot-conf/
magickdir=${d}/../imagemagick/

infile=${1}
tile_width=${2}
tile_height=${3}
output_dir=${4}

function log {
    logfile=${bindir}/data/log/preprocess_mosaic.log
    if [ ! -f logfile ] ; then
        touch ${logfile}
    fi

    dat=$(date +%Y.%m.%d.%H.%M.%S)
    echo "${dat}: ${1}" >> ${logfile}
}

# Make sure the file exists.
if [ ! -f ${infile} ] ; then
    log "Cannot find ${infile}"
    exit
fi

filename=$(basename "$infile")
extension="${filename##*.}"
filename="${filename%.*}"

resized_filepath=${output_dir}/${filename}.png

# Resize to tile format
${magickdir}/convert ${infile} \
    -resize ${tile_width}x${tile_height}^ \
    -gravity center \
    -extent ${tile_width}x${tile_height} \
    -colors 256 \
    PNG24:${resized_filepath}

rm ${infile}

