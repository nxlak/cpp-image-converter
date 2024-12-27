# ImgLib/CMakeLists.txt
cmake_minimum_required(VERSION 3.11)

project(ImgLib CXX)
set(CMAKE_CXX_STANDARD 17)

set(LIBJPEG_DIR CACHE STRING "LibJPEG static library directory")
if(NOT LIBJPEG_DIR)
    message(FATAL_ERROR "Please, specify LibJPEG directory via -DLIBJPEG_DIR=<dir>")
endif()
message(STATUS "LibJPEG dir is ${LIBJPEG_DIR}, change via -DLIBJPEG_DIR=<dir>")

set(IMGLIB_MAIN_FILES img_lib.h img_lib.cpp)

set(IMGLIB_FORMAT_FILES 
    ppm_image.h ppm_image.cpp 
    jpeg_image.h jpeg_image.cpp
    bmp_image.h bmp_image.cpp)

add_library(ImgLib STATIC ${IMGLIB_MAIN_FILES} 
            ${IMGLIB_FORMAT_FILES})

target_include_directories(ImgLib PUBLIC "${LIBJPEG_DIR}/include")

target_link_directories(ImgLib INTERFACE
    "${LIBJPEG_DIR}/lib/$<IF:$<CONFIG:Debug>,Debug,Release>"
    )

target_link_libraries(ImgLib INTERFACE jpeg)