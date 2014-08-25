#!/bin/sh

d=${PWD}

if [ ! -d build.release ] ; then 
    mkdir build.release
fi



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

    # Create input image dir for resized images
    if [ ! -d ${id}/bin/data/input_resized ] ; then 
        mkdir -p ${id}/bin/data/input_resized
    fi

    # Create input image dir for blurred images
    if [ ! -d ${id}/bin/data/input_blurred ] ; then 
        mkdir -p ${id}/bin/data/input_blurred
    fi

else
    cd ./../../install/linux-gcc-x86_64/bin/
fi

#./test_fex_load_image
#./test_libav_rtmp
#./test_video_stream_player
#./test_fex
#./test_offline_analyzer
./test_online_analyzer
