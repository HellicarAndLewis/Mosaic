#!/bin/bash
#set -x
d=${PWD}
sd=${d}/mac-sources
bd=${d}/../extern/mac-clang-x86_64
id=${d}/../install/mac-clang-x86_64

export PATH=${PATH}:${bd}/bin/:${sd}/gyp/
export CFLAGS="-I\"${bd}/include\""
export LDFLAGS="-L\"${bd}/lib\""

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
if [ ! -f ${id}/lib/libMagick++-6.Q16.3.dylib ] ; then
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
if [ -f $sd}/yasm.tar.gz ] ; then 
    rm ${sd}/yasm.tar.gz
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
    mkdir build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=${bd} ../
    cmake --build . --target install
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
