#!/bin/bash
set -x
d=${PWD}
sd=${d}/mac-sources
bd=${d}/../extern/mac-clang-x86_64
id=${d}/../install/mac-clang-x86_64

export PATH=${PATH}:${bd}/bin/:${sd}/gyp/
export CFLAGS="-I${bd}/include"
export LDFLAGS="-L${bd}/lib"
cfcopy=${CFLAGS}
ldcopy=${LDFLAGS}


# ----------------------------------------------------------------------- #
#                D O W N L O A D   D E P E N D E N C I E S 
# ----------------------------------------------------------------------- #
if [ ! -d ${sd} ] ; then 
    mkdir -p ${sd}
fi

if [ ! -d ${bd} ] ; then
    mkdir -p ${bd}
fi

if [ ! -d ${bd}/src ] ; then 
    mkdir -p ${bd}/src
fi

if [ ! -d ${bd}/include ] ; then 
    mkdir -p ${bd}/include
fi

# Download autoconf and friends (for libuv)
if [ ! -d ${sd}/autoconf ] ; then 
    cd ${sd}
    curl -o autoconf.tar.gz http://ftp.gnu.org/gnu/autoconf/autoconf-2.69.tar.gz
    tar -zxvf autoconf.tar.gz
    mv autoconf-2.69 autoconf
fi 

# Download libtool
if [ ! -d ${sd}/libtool ] ; then
    cd ${sd}
    curl -o libtool.tar.gz http://ftp.gnu.org/gnu/libtool/libtool-2.4.2.tar.gz
    tar -zxvf libtool.tar.gz
    mv libtool-2.4.2 libtool
fi

# Download automake
if [ ! -d ${sd}/automake ] ; then
    cd ${sd}
    curl -o automake.tar.gz http://ftp.gnu.org/gnu/automake/automake-1.14.tar.gz
    tar -zxvf automake.tar.gz
    mv automake-1.14 automake
fi

# Download libuv
if [ ! -d ${sd}/libuv ] ; then
    cd ${sd}
    git clone https://github.com/joyent/libuv.git libuv
fi

# Download gyp for libuv
if [ ! -d ${sd}/libuv/build/gyp ] ; then 
    cd ${sd}/libuv
    git clone https://git.chromium.org/external/gyp.git build/gyp
fi

# Download libz
if [ ! -d ${sd}/zlib ] ; then
    cd ${sd}
    if [ ! -f libz.tar.gz ] ; then
        curl -o libz.tar.gz http://zlib.net/zlib-1.2.8.tar.gz
        tar -zxvf libz.tar.gz
    fi
    mv zlib-1.2.8 zlib
fi

# Download mongoose (signaling)
if [ ! -d ${sd}/mongoose ] ; then 
    cd ${sd}
    git clone https://github.com/cesanta/mongoose.git mongoose
fi    
  
if [ ! -f ${bd}/src/mongoose.c ] ; then
    cp ${sd}/mongoose/mongoose.c ${bd}/src/
    cp ${sd}/mongoose/mongoose.h ${bd}/include/
fi

# Download net_skeleton (signaling)
if [ ! -d ${sd}/net_skeleton ] ; then 
    cd ${sd}
    git clone https://github.com/cesanta/net_skeleton.git net_skeleton
fi

if [ ! -f ${bd}/src/net_skeleton.c ] ; then
    cp ${sd}/net_skeleton/net_skeleton.c ${bd}/src/
    cp ${sd}/net_skeleton/net_skeleton.h ${bd}/include/
fi

# Download ssl_wrapper (signaling)
if [ ! -d ${sd}/ssl_wrapper ] ; then 
    cd ${sd}
    git clone https://github.com/cesanta/ssl_wrapper.git ssl_wrapper
fi

if [ ! -f ${bd}/src/ssl_wrapper.c ] ; then
    cp ${sd}/ssl_wrapper/ssl_wrapper.c ${bd}/src/
    cp ${sd}/ssl_wrapper/ssl_wrapper.h ${bd}/include/
fi

# Download libpng
if [ ! -d ${sd}/libpng ] ; then 
    cd ${sd}
    if [ ! -f libpng.tar.gz ] ; then 
        curl -o libpng.tar.gz -L http://prdownloads.sourceforge.net/libpng/libpng-1.2.51.tar.gz?download
        tar -zxvf libpng.tar.gz
    fi
    mv libpng-1.2.51 libpng
fi

# Download rapidxml
if [ ! -d ${sd}/rapidxml ] ; then 
    cd ${sd}
    curl -o rapidxml.zip -L "https://sourceforge.net/projects/rapidxml/files/rapidxml/rapidxml%201.13/rapidxml-1.13.zip/download"
    unzip rapidxml.zip
    mv rapidxml-1.13 rapidxml
fi 

# Download libjpg
if [ ! -d ${sd}/libjpeg ] ; then 
    cd ${sd}
    curl -o jpeg.tar.gz http://www.ijg.org/files/jpegsrc.v9a.tar.gz
    tar -zxvf jpeg.tar.gz
    mv jpeg-9a libjpeg
fi 

# Download GLAD for GL
if [ ! -d ${sd}/glad ] ; then 
    cd ${sd}
    git clone https://github.com/Dav1dde/glad.git glad
fi

# Download GLFW for GL
if [ ! -d ${sd}/glfw ] ; then 
    cd ${sd}
    git clone https://github.com/glfw/glfw.git glfw
fi

# Download the tinylib 
if [ ! -d ${d}/../extern/tinylib ] ; then 
    mkdir ${d}/../extern/tinylib
    cd ${d}/../extern/tinylib
    git clone git@github.com:roxlu/tinylib.git .
fi

# Downoad video capture library
if [ ! -d ${d}/../extern/video_capture ] ; then
    mkdir ${d}/../extern/video_capture
    cd ${d}/../extern/video_capture
    git clone git@github.com:roxlu/video_capture.git .
fi

# Download ImageMagick
if [ ! -d ${sd}/imagemagick ] ; then
    cd ${sd}
    curl -o imagemagick.tar.gz ftp://ftp.imagemagick.org/pub/ImageMagick/binaries/ImageMagick-x86_64-apple-darwin13.2.0.tar.gz
    tar -zxvf imagemagick.tar.gz
    mv ImageMagick-6.8.9 imagemagick
fi

# Fix ImageMagick dylibs + install
if [ ! -f ${id}/imagemagick/convert ] ; then
    if [ ! -d ${id} ] ; then 
        mkdir ${id}
    fi
    if [ ! -d ${id}/lib ] ; then 
        mkdir ${id}/lib
    fi
    if [ ! -d ${id}/imagemagick ] ; then 
        mkdir ${id}/imagemagick
    fi

    # fix dylib paths for imagemagick apps
    cd ${sd}/imagemagick/lib
    for dylib in `ls -1 *.dylib`; do
        for app in ${sd}/imagemagick/bin/* ; do 
            install_name_tool -change "/ImageMagick-6.8.9/lib/${dylib}" "@executable_path/../lib/${dylib}" ${app}
            cp ${app} ${id}/imagemagick/
        done
    done
    
    # fix dylib paths for the dylibs themself + copy them
    cd ${sd}/imagemagick/lib
    for dylib_a in `ls -1 *.dylib`; do
        for dylib_b in `ls -1 *.dylib`; do
            if [ "${dylib_a}" == "${dylib_b}" ] ; then
                echo "${dylib_a} == ${dylib_b}"
            else
                install_name_tool -change /ImageMagick-6.8.9/lib/${dylib_b} "@executable_path/../lib/${dylib_b}" ${dylib_a}
            fi
        done
        cp ${sd}/imagemagick/lib/${dylib_a} ${id}/lib
    done
fi

# Download libav
if [ ! -d ${sd}/libav ] ; then
    cd ${sd}
    git clone git://git.libav.org/libav.git libav
fi

# Download yasm, needed for libvpx
if [ ! -d ${sd}/yasm ] ; then
    cd ${sd}
    curl -o yasm.tar.gz http://www.tortall.net/projects/yasm/releases/yasm-1.3.0.tar.gz
    tar -zxvf yasm.tar.gz
    mv yasm-1.3.0 yasm
fi

# Download microprofile
if [ ! -d ${sd}/microprofile ] ; then
    cd ${sd}
    hg clone https://bitbucket.org/jonasmeyer/microprofile 
fi

# Download ogg for rxp_player
if [ ! -d ${sd}/libogg ] ; then
    cd ${sd}
    curl -o libogg.tar.gz http://downloads.xiph.org/releases/ogg/libogg-1.3.1.tar.gz
    tar -zxvf libogg.tar.gz
    mv libogg-1.3.1 libogg
fi

# Download theora
if [ ! -d ${sd}/theora ] ; then
    cd ${sd}
    # git clone https://git.xiph.org/mirrors/theora.git  # configure error on mac, uses invalid flags.
    #curl -o theora.zip http://downloads.xiph.org/releases/theora/libtheora-1.1.1.zip
    #unzip theora
    #mv libtheora-1.1.1 theora

    svn co http://svn.xiph.org/trunk/theora
fi     

# Downoad vorbis 
if [ ! -d ${sd}/vorbis ] ; then
    cd ${sd}
    curl -o vorbis.tar.gz http://downloads.xiph.org/releases/vorbis/libvorbis-1.3.3.tar.gz
    tar -zxvf vorbis.tar.gz
    mv libvorbis-1.3.3 vorbis
fi

# Download rxp_player for video playback
if [ ! -d ${sd}/rxp_player ] ; then 
    cd ${sd}
    git clone git@github.com:roxlu/rxp_player.git
fi 

# Download the tracker lib
if [ ! -d ${d}/../extern/tracker ] ; then 
    cd ${d}/../extern/
    git clone git@github.com:roxlu/tracker.git
fi 

# Download opencv for block tracking
if [ ! -d ${sd}/opencv ] ; then 
    cd ${sd}
    if [ ! -f opencv.zip ] ; then
        curl -L -o opencv.zip https://github.com/Itseez/opencv/archive/3.0.0-alpha.zip
    fi
    unzip opencv.zip
    mv opencv-3.0.0-alpha opencv
fi
  
# Download tcmalloc
# if [ ! -d ${sd}/tcmalloc ] ; then
#     cd ${sd}
#     mkdir tcmalloc
#     cd tcmalloc
#     git clone https://code.google.com/p/gperftools/ .
# fi

# Cleanup some files we don't need anymore.
if [ -f ${sd}/autoconf.tar.gz ] ; then
    rm ${sd}/autoconf.tar.gz
fi
if [ -f ${sd}/automake.tar.gz ] ; then
    rm ${sd}/automake.tar.gz
fi
if [ -f ${sd}/libtool.tar.gz ] ; then
    rm ${sd}/libtool.tar.gz 
fi
if [ -f ${sd}/libz.tar.gz ] ; then
    rm ${sd}/libz.tar.gz 
fi
if [ -f ${sd}/libpng.tar.gz ] ; then 
    rm ${sd}/libpng.tar.gz
fi
if [ -f ${sd}/jpeg.tar.gz ] ; then
    rm ${sd}/jpeg.tar.gz
fi 
if [ -f ${sd}/imagemagick.tar.gz ] ; then 
    rm ${sd}/imagemagick.tar.gz
fi 

if [ -f ${sd}/yasm.tar.gz ] ; then 
    rm ${sd}/yasm.tar.gz
fi 
if [ -f ${sd}/rapidxml.zip ] ; then
    rm ${sd}/rapidxml.zip
fi
if [ -f ${sd}/libogg.tar.gz ] ; then
    rm ${sd}/libogg.tar.gz
fi
if [ -f ${sd}/theora.zip ] ; then
    rm ${sd}/theora.zip
fi
if [ -f ${sd}/vorbis.tar.gz ] ; then
    rm ${sd}/vorbis.tar.gz
fi
# ----------------------------------------------------------------------- #
#                C O M P I L E   D E P E N D E N C I E S 
# ----------------------------------------------------------------------- #

# Compile autoconf
if [ ! -f ${bd}/bin/autoconf ] ; then
    cd ${sd}/autoconf
    ./configure --prefix=${bd}
    make
    make install
fi

# Compile libtool
if [ ! -f ${bd}/bin/libtool ] ; then
    cd ${sd}/libtool
    ./configure --prefix=${bd}
    make
    make install
fi

# Compile automake 
if [ ! -f ${bd}/bin/automake ] ; then 
    cd ${sd}/automake
    ./configure --prefix=${bd}
    make
    make install
fi

# Compile libuv
if [ ! -f ${bd}/lib/libuv.a ] ; then
    cd ${sd}/libuv
    ./gyp_uv.py -f xcode
    xcodebuild -ARCHS="x86_64" -project uv.xcodeproj -configuration Release -target All
    cp ${sd}/libuv/build/Release/libuv.a ${bd}/lib/
    cp ${sd}/libuv/include/*.h ${bd}/include/
fi

# Compile zlib
if [ ! -f ${bd}/lib/libz.a ] ; then 
    cd ${sd}/zlib
    ./configure --prefix=${bd} --static --64
    make
    make install
fi

# Compile libpng
if [ ! -f ${bd}/lib/libpng.a ] ; then 
    cd ${sd}/libpng
    ./configure --enable-static --prefix=${bd}
    make
    make install
fi

# Compile libjpeg
if [ ! -f ${bd}/lib/libjpeg.a ] ; then 
    cd ${sd}/libjpeg
    ./configure --prefix=${bd}
    make 
    make install
fi

# Copy the GLAD sources + generate the C extension
if [ ! -f ${bd}/src/glad.c ] ; then
    if [ ! -d ${bd}/src ] ; then
        mkdir ${bd}/src 
    fi
    cd ${sd}/glad
    python main.py --generator=c --out-path=gl --extensions GL_ARB_timer_query
    cp -r ${sd}/glad/gl/include/glad ${bd}/include/
    cp -r ${sd}/glad/gl/include/KHR ${bd}/include/
    cp ${sd}/glad/gl/src/glad.c ${bd}/src/
fi

# Compile glfw
if [ ! -f ${bd}/lib/libglfw3.a ] ; then
    cd ${sd}/glfw
    if [ -d build ] ; then 
        rm -r build
    fi
    if [ ! -d build ] ; then
        mkdir build
    fi

    cfcopy=${CFLAGS}
    ldcopy=${LDFLAGS}
    export CFLAGS=""
    export LDFLAGS=""

    cd build
    cmake -DCMAKE_INSTALL_PREFIX=${bd} ..
    cmake --build . --target install

    export CFLAGS=${cfcopy}
    export LDFLAGS=${ldcopy}
fi

# Compile yasm
if [ ! -f ${bd}/bin/yasm ] ; then
    cd ${sd}/yasm
    ./configure --prefix=${bd}
    make
    make install
fi

# Compile libav 
if [ ! -f ${bd}/lib/libavcodec.a ] ; then
    cd ${sd}/libav
    ./configure --prefix=${bd} --enable-gpl 
    make
    make install
fi

# Move rapid xml sources 
if [ ! -f ${bd}/include/rapidxml_iterators.hpp ] ; then
    cd ${sd}/rapidxml
    cp rapidxml_iterators.hpp ${bd}/include/
    cp rapidxml_print.hpp ${bd}/include/
    cp rapidxml_utils.hpp ${bd}/include/
    cp rapidxml.hpp ${bd}/include/
fi

# Move the microprofiler
if [ ! -f ${bd}/include/microprofile.h ] ; then
    cp ${sd}/microprofile/microprofile.h ${bd}/include
    cp ${sd}/microprofile/demo/ui/microprofile.cpp ${bd}/src
fi

# Compile ogg
if [ ! -f ${bd}/lib/libogg.a ] ; then
    cd ${sd}/libogg
    ./configure --prefix=${bd}
    make
    make install
fi

# Compile vorbis
if [ ! -f ${bd}/lib/libvorbis.a ] ; then
    cd ${sd}/vorbis
    ./configure --prefix=${bd}
    make
    make install
fi

# Compile libtheora
if [ ! -f ${bd}/lib/libtheora.a ] ; then 
    cd ${sd}/theora
    ./autogen.sh
    ./configure --prefix=${bd} 
    make
    make install
fi

# Compile rxp_player
if [ ! -f ${bd}/lib/rxp_player.a ] ; then
    cd ${sd}/rxp_player/build
    mkdir build.release
    cd build.release
    cmake -DCMAKE_INSTALL_PREFIX=${bd} -DCMAKE_BUILD_TYPE=Release ..
    cmake --build . --target install
fi

# Compile opencv
if [ ! -f ${bd}/lib/libopencv_core.a ] ; then 
    cd ${sd}/opencv
    if [ ! -d build.release ] ; then
        mkdir build.release
    fi
    cd build.release
    cmake -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=${bd} \
        -DBUILD_SHARED_LIBS=0 \
        -DBUILD_PACKAGE=0 \
        -DBUILD_PERF_TESTS=0 \
        -DBUILD_PNG=0 \
        -DBUILD_TBB=0 \
        -DBUILD_TESTS=0 \
        -DBUILD_TIFF=0 \
        -DBUILD_WITH_DEBUG_INFO=0 \
        -DBUILD_ZLIB=0 \
        -DBUILD_EXAMPLES=1 \
        -DBUILD_opencv_apps=0 \
        -DBUILD_opencv_bioinspired=0 \
        -DBUILD_opencv_calib3d=0 \
        -DBUILD_opencv_contrib=0 \
        -DBUILD_opencv_core=1 \
        -DBUILD_opencv_cuda=0, \
        -DBUILD_opencv_features2d=1 \
        -DBUILD_opencv_flann=1 \
        -DBUILD_opencv_highgui=0 \
        -DBUILD_opencv_imgproc=1 \
        -DBUILD_opencv_legacy=0 \
        -DBUILD_opencv_ml=1 \
        -DBUILD_opencv_nonfree=0 \
        -DBUILD_opencv_objdetect=1 \
        -DBUILD_opencv_ocl=1 \
        -DBUILD_opencv_optim=1 \
        -DBUILD_opencv_photo=1 \
        -DBUILD_opencv_python=0 \
        -DBUILD_opencv_python2=0 \
        -DBUILD_opencv_shape=0 \
        -DBUILD_opencv_softcascade=0 \
        -DBUILD_opencv_stitching=0 \
        -DBUILD_opencv_video=1 \
        -DBUILD_opencv_videostab=0 \
        -DBUILD_opencv_world=0 \
        -DWITH_CUDA=0 \
        -DWITH_CUFFT=0 \
        -DWITH_EIGEN=0 \
        -DWITH_JPEG=0 \
        -DWITH_JASPER=0 \
        -DWITH_LIBV4L=0 \
        -DWITH_OPENCL=1 \
        -DWITH_OPENEXR=0 \
        -DWITH_PNG=0 \
        -DWITH_TIFF=0 \
        -DWITH_V4L=0 \
        -DWITH_WEBP=0 \
        -DWITH_QT=0 \
        -DWITH_FFMPEG=0 \
        -DWITH_VTK=0 \
        -DWITH_IPP=0 \
        ..

    cmake --build . --target install
fi

# Compile tcmalloc
# if [ ! -f ${bd}/lib/libtcmalloc.a ] ; then
#     cd ${sd}/tcmalloc
#     ./autogen.sh
#     ./configure --prefix=${bd} CC=clang CXX=clang++ CXXFLAGS=-fno-builtin
#     make
#     make install
# fi
