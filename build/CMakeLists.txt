cmake_minimum_required(VERSION 2.8)

set(USE_OPENGL OFF)
include(${CMAKE_CURRENT_LIST_DIR}/../extern/video_capture/build/CMakeLists.txt)
include(${CMAKE_CURRENT_LIST_DIR}/Triplet.cmake)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

set(bd ${CMAKE_CURRENT_LIST_DIR}/../)
set(fex_sd ${bd}/projects/featurex/src) 
set(gfx_sd ${bd}/projects/gfx/src)
set(vid_sd ${bd}/projects/video/src)
set(mos_sd ${bd}/projects/mosaic/src)
set(grid_sd ${bd}/projects/grid/src)

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}/../src
  ${CMAKE_CURRENT_LIST_DIR}/../include
  ${extern_include_dir}
  ${CMAKE_CURRENT_LIST_DIR}/../extern/tinylib/src
  ${bd}/projects/featurex/include
  ${bd}/projects/gfx/include
  ${bd}/projects/video/include
  ${bd}/projects/mosaic/include
  ${bd}/projects/grid/include
  )

if(APPLE)
  find_library(fr_corefoundation CoreFoundation)
  find_library(fr_avfoundation AVFoundation)
  find_library(fr_coremedia CoreMedia)
  find_library(fr_cocoa Cocoa)
  find_library(fr_opengl OpenGL)
  find_library(fr_iokit IOKit)
  find_library(fr_corevideo CoreVideo)
  find_library(fr_opencl OpenCL)
  find_library(fr_audio AudioUnit)
  find_library(fr_caudio CoreAudio)
  find_library(fr_audio_toolbox AudioToolbox)

  set(app_libs
    ${extern_lib_dir}/libuv.a
    ${extern_lib_dir}/libpng.a
    ${extern_lib_dir}/libjpeg.a
    ${extern_lib_dir}/libz.a
    ${extern_lib_dir}/libglfw3.a

    ${extern_lib_dir}/libavcodec.a
    ${extern_lib_dir}/libavformat.a
    ${extern_lib_dir}/libavutil.a
    ${extern_lib_dir}/libavresample.a

    ${fr_corefoundation}               # uv
    ${fr_cocoa}                        # uv
    ${fr_iokit}                        # uv
    ${fr_opengl}                       # glfw
    ${fr_corevideo}                    # glfw
    ${fr_avfoundation}                 # videocapture
    ${fr_coremedia}                    # videocapture

    bz2 
    )
elseif(UNIX)

endif()

# Feature Extractor and Matching library
set(fex_lib_sources
  ${fex_sd}/Featurex.cpp
  ${fex_sd}/AverageColorGPU.cpp
  ${fex_sd}/Config.cpp
  ${fex_sd}/Descriptor.cpp
  ${fex_sd}/Comparator.cpp
  ${fex_sd}/AnalyzerCPU.cpp
  ${fex_sd}/AnalyzerGPU.cpp
  ${fex_sd}/TilesPool.cpp
)

# OpenGL wrapper/utilss.
set(gfx_lib_sources
  ${gfx_sd}/Image.cpp
  ${gfx_sd}/Blur.cpp
  ${gfx_sd}/FBO.cpp
  ${gfx_sd}/BlurFBO.cpp
  ${gfx_sd}/Timer.cpp
  ${gfx_sd}/AsyncDownload.cpp
  ${gfx_sd}/AsyncUpload.cpp
)

# Video playback of RTMP
set(vid_lib_sources
  ${vid_sd}/Stream.cpp
  ${vid_sd}/Player.cpp
  ${vid_sd}/Jitter.cpp
  ${vid_sd}/YUV420P.cpp
)

# The grid
set(grid_lib_sources
  ${grid_sd}/Grid.cpp
  )

# The mosaic visualizer (uses all of above)
set(mosaic_lib_sources
  ${mos_sd}/Mosaic.cpp
  ${mos_sd}/Config.cpp
  ${mos_sd}/VideoInput.cpp
  ${mos_sd}/ImageLoader.cpp
  ${mos_sd}/DirWatcher.cpp
)

macro(create_test name)
  add_executable(test_${name} 
    ${bd}/projects/tests/test_${name}.cpp 
    ${extern_source_dir}/glad.c
    )
  target_link_libraries(test_${name} ${app_libs} gfx fex vid mosaic grid videocapture)
  install(TARGETS test_${name} DESTINATION bin)
endmacro()

# The graphics library
add_library(gfx ${gfx_lib_sources})
install(TARGETS gfx ARCHIVE DESTINATION lib)

# The feature-extractor library
add_library(fex ${fex_lib_sources})
install(TARGETS fex ARCHIVE DESTINATION lib)

# The video library 
add_library(vid ${vid_lib_sources})
install(TARGETS vid ARCHIVE DESTINATION lib)

# The grid library 
add_library(grid ${grid_lib_sources})
install(TARGETS grid ARCHIVE DESTINATION lib)

# The mosaic library
add_library(mosaic ${mosaic_lib_sources})
install(TARGETS mosaic ARCHIVE DESTINATION lib)

# The tests used during development
create_test(mosaic)
#create_test(fex_load_image)
#create_test(libav_rtmp)
#create_test(video_stream_player)
#create_test(offline_analyzer)
#create_test(online_analyzer)
#create_test(async_upload)
#create_test(image_loader)
#create_test(grid)
#create_test(png_rgba)







