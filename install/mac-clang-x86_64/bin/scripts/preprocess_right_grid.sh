#!/bin/sh
#set -x
d=${MOSAIC_BINDIR} # MOSAIC_BINDIR can be set in /etc/launchd.conf when using launch agents.
if [ -z "${d}" ] ; then
    if [ -f ${PWD}/AppGridLeft ] ; then
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
raw_mosaic_dir=${5}
polaroid_dir=${6}
username=${7}

function log {
    logfile=${bindir}/data/log/preprocess_right.log
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
tmp_filename="${d}/tmp_${filename}.png"
png_filename="${d}png_${filename}.png"
grid_filepath=${output_dir}/${filename}.png

# Convert to png and make sure the input image is 640x640 because the positioning depends on these dimensions
${magickdir}/convert ${infile} \
    -resize 640x640^ \
    -gravity center \
    ${png_filename}

# Small Polaroid for left grid.
${bindir}/AppPolaroid \
    -x 12 -y 60 -f ${png_filename} \
    -r 0.0 -g 0.0 -b 0.0 \
    -n "${username} " -s 35 -t 21 -w 13 \
    -h "#TOPSHOPWINDOW" -i 32 -j 210 \
    -a 180 -c ${bindir}/data/assets/polaroid_overlay_small.png \
    -o ${tmp_filename}
cp ${tmp_filename} ${grid_filepath}

# Small polaroid for interaction
${bindir}/AppPolaroid \
    -x 130 -y 115 -f ${png_filename} \
     -r 0.0 -g 0.0 -b 0.0 \
     -n "${username}" -s 66 -t 37 -w 13 \
     -h "#TOPSHOPWINDOW" -i 63 -j 226 \
     -a 180 -c ${bindir}/data/assets/polaroid_overlay_small_for_interaction.png \
     -o ${tmp_filename}
cp ${tmp_filename} ${polaroid_dir}/${filename}.png

# Move to mosaic dir
mv ${infile} ${raw_mosaic_dir}/${filename}.${extension}
rm ${png_filename}
rm ${tmp_filename}
