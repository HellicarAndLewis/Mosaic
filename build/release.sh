#!/bin/sh

d=${PWD}

if [ ! -d build.release ] ; then 
    mkdir build.release
fi

# Make sure the tinylib.h files are in sync.
# if [ -f ${d}/../extern/video_capture/shared/tinylib/src/tinylib.h ] ; then
#     cp ${d}/../extern/tinylib/src/tinylib.h ${d}/../extern/video_capture/shared/tinylib/src/tinylib.h
# fi

./build_mac_dependencies.sh

cd build.release
cmake -DCMAKE_BUILD_TYPE=Release ../ 
#cmake -DCMAKE_BUILD_TYPE=Debug ../ 
cmake --build . --target install

if [ "$(uname)" == "Darwin" ] ; then 
    cd ./../../install/mac-clang-x86_64/bin/

    # Create log dir.
    id=${d}/../install/mac-clang-x86_64
    if [ ! -d ${id}/bin/log ] ; then 
        mkdir ${id}/bin/log
    fi
    if [ ! -d ${id}/bin/data/input_blurred ] ; then 
        mkdir -p ${id}/bin/data/input_blurred
    fi
    if [ ! -d ${id}/bin/data/input_grid_left ] ; then 
        mkdir -p ${id}/bin/data/input_grid_left
    fi
    if [ ! -d ${id}/bin/data/input_grid_right ] ; then 
        mkdir -p ${id}/bin/data/input_grid_right
    fi
    if [ ! -d ${id}/bin/data/raw_left ] ; then 
        mkdir -p ${id}/bin/data/raw_left
    fi
    if [ ! -d ${id}/bin/data/raw_right ] ; then 
        mkdir -p ${id}/bin/data/raw_right
    fi
    if [ ! -d ${id}/bin/data/raw_mosaic ] ; then 
        mkdir -p ${id}/bin/data/raw_mosaic
    fi

else
    cd ./../../install/linux-gcc-x86_64/bin/
fi

#./test_fex_load_image
#./test_libav_rtmp
#./test_video_stream_player
#./test_mosaic
#./test_offline_analyzer
#./test_online_analyzer
#./test_async_upload
#./test_image_loader
#./test_grid
#./test_png_rgba
#./test_ogg_player

./TopShop
