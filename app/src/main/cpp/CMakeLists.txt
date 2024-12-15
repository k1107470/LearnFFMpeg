
cmake_minimum_required(VERSION 3.22.1)

#打印当前 cmakeLists 的路径
message("当前cmakel路径: ${CMAKE_SOURCE_DIR} \n cpu架构：${CMAKE_ANDROID_ARCH_ABI}")


set(jnilibs ${CMAKE_SOURCE_DIR}/../jniLibs)
set(libname learn-ffmpeg)




# 引入FFmpeg的头文件
include_directories(
        ${CMAKE_SOURCE_DIR}/include
        ${CMAKE_SOURCE_DIR}/decoder
        ${CMAKE_SOURCE_DIR}/demuxer
        ${CMAKE_SOURCE_DIR}/audio
        ${CMAKE_SOURCE_DIR}/video
        ${CMAKE_SOURCE_DIR}/video/EGL
        ${CMAKE_SOURCE_DIR}/video/shader
        ${CMAKE_SOURCE_DIR}/video/texture
        ${CMAKE_SOURCE_DIR}/video/view
        ${CMAKE_SOURCE_DIR}/util
        ${CMAKE_SOURCE_DIR}/util/Log
        ${CMAKE_SOURCE_DIR}/util/thread
        ${CMAKE_SOURCE_DIR}/util/observer
)

link_directories(${jnilibs}/${CMAKE_ANDROID_ARCH_ABI})


file(GLOB_RECURSE  src-files ${CMAKE_SOURCE_DIR}/*.cpp)
#file(GLOB  src-files ${CMAKE_SOURCE_DIR}/*.cpp)

add_library(
        ${libname}
        SHARED
        ${src-files}
)

set(third-party-libs
        avformat
        avcodec
        avfilter
        swresample
        swscale
        avutil
)

set(native-libs
    android
        #mediandk
        EGL
        GLESv2
        OpenSLES
        Log
        m
        z
)

target_link_libraries(
        ${libname}
        ${log-lib}
        ${third-party-libs}
        ${native-libs}
)
