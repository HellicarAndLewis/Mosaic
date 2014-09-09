#!/bin/sh
#set -x
d=${PWD}
magickdir=${d}/../imagemagick/
infile=${1}
tile_width=${2}
tile_height=${3}
output_dir=${4}
raw_mosaic_dir=${5}
polaroid_dir=${6}
username=${7}

function log {
    dat=$(date +%Y.%m.%d.%H.%M.%S)
    echo "${dat}: ${1}" >> data/log/preprocess_right.log
}

# Make sure the file exists.
if [ ! -f ${infile} ] ; then
    log "Cannot find ${infile}"
    exit
fi

filename=$(basename "$infile")
extension="${filename##*.}"
filename="${filename%.*}"
tmp_filename="tmp_${filename}.png"
png_filename="png_${filename}.png"
grid_filepath=${output_dir}/${filename}.png

# Convert to png
${magickdir}/convert ${infile} ${png_filename}

# Small Polaroid for left grid.
./AppPolaroid \
    -x 35 -y 10 -f ${png_filename} \
    -r 0.0 -g 0.0 -b 0.0 \
    -n "${username} " -s 12 -t 15 -w 13 \
    -h "#TOPSHOPWINDOW" -i 11 -j 193 \
    -a 180 -c ./data/assets/polaroid_overlay_small.png \
    -o ${tmp_filename}
cp ${tmp_filename} ${grid_filepath}

# Small polaroid for interaction
./AppPolaroid \
    -x 130 -y 115 -f ${png_filename} \
    -r 0.0 -g 0.0 -b 0.0 \
    -n "${username}" -s 36 -t 42 -w 13 \
    -h "#TOPSHOPWINDOW" -i 36 -j 221 \
    -a 180 -c ./data/assets/polaroid_overlay_small_for_interaction.png \
    -o ${tmp_filename}
cp ${tmp_filename} ${polaroid_dir}/${filename}.png

# Move to mosaic dir
mv ${infile} ${raw_mosaic_dir}/${filename}.${extension}
rm ${png_filename}
rm ${tmp_filename}

# Resize 
# ${magickdir}/convert ${infile} \
#     -resize ${tile_width}x${tile_height}^ \
#     -gravity center \
#     -extent ${tile_width}x${tile_height} \
#     -colors 256 \
#     PNG8:tmp.png

