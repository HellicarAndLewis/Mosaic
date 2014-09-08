#!/bin/sh

d=${PWD}

if [ ! -d build.release ] ; then 
    mkdir build.release
fi


# if [ -f ${d}/../extern/video_capture/shared/tinylib/src/tinylib.h ] ; then
#     cp ${d}/../extern/tinylib/src/tinylib.h ${d}/../extern/video_capture/shared/tinylib/src/tinylib.h
# fi

./build_mac_dependencies.sh

cd build.release
cmake -DCMAKE_BUILD_TYPE=Release -DOPT_BUILD_TRACKER_LIB=On ../ 
#cmake -DCMAKE_BUILD_TYPE=Debug ../ 
cmake --build . --target install

if [ "$(uname)" == "Darwin" ] ; then 
    cd ./../../install/mac-clang-x86_64/bin/

    # Create log dir.
    id=${d}/../install/mac-clang-x86_64
    if [ ! -d ${id}/bin/data/log_left ] ; then 
        mkdir ${id}/bin/data/log_left
    fi
    if [ ! -d ${id}/bin/data/log_right ] ; then 
        mkdir ${id}/bin/data/log_right
    fi
    if [ ! -d ${id}/bin/data/log_mosaic ] ; then 
        mkdir ${id}/bin/data/log_mosaic
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

#./test/test_fex_load_image
#./test/test_libav_rtmp
#./test/test_video_stream_player
#./test/test_mosaic
#./test/test_offline_analyzer
#./test/test_online_analyzer
#./test/test_async_upload
#./test/test_image_loader
#./test/test_grid
#./test/test_png_rgba
#./test/test_ogg_player
#./test/test_tracker
#./test/test_cairo
#./test/test_image_json
#./test/test_cairo_jpg
./AppMosaic
#./AppGridLeft
#./AppGridRight

