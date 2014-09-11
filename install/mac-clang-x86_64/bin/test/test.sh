#!/bin/sh

# Interactive version
./../AppPolaroid \
    -x 130 -y 115 -f infile.png \
    -r 0.0 -g 0.0 -b 0.0 \
    -n "USERNAME" -s 66 -t 37 -w 13 \
    -h "#TOPSHOPWINDOW" -i 63 -j 226 \
    -a 180 -c ./../data/assets/polaroid_overlay_small_for_interaction.png \
    -o out.png

exit 

# Grid version
./../AppPolaroid \
    -x 12 -y 60 -f infile.png \
    -r 0.0 -g 0.0 -b 0.0 \
    -n "USERNAME" -s 35 -t 21 -w 13 \
    -h "#TOPSHOPWINDOW" -i 32 -j 210 \
    -a 180 -c ./../data/assets/polaroid_overlay_small.png \
    -o out.png



#     -r 0.247 -g 0.447 -b 0.608 \
#     -s 10 -t 192 -w 10 \ 
