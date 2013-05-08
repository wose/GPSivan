cmake_minimum_required(VERSION 2.6)
project(GPSivan)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /opt/vc/lib)
set(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} /opt/vc/include)

find_package(GPSD REQUIRED)
find_package(PNG REQUIRED)
find_package(EGL REQUIRED)
find_package(GLES REQUIRED)
find_package(KAZMATH REQUIRED)

include_directories(
        /opt/vc/include/interface/vcos/pthreads
       /opt/vc/include/interface/vmcs_host/linux)
include_directories(${GPSD_INCLUDE_DIR})
include_directories(${ONG_INCLUDE_DIR})
include_directories(${EGL_INCLUDE_DIR})
include_directories(${OPENGLES2_INCLUDE_DIR})
include_directories(${KAZMATH_INCLUDE_DIR})
include_directories(/usr/local/include)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/include")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src")

link_directories(${LINK_DIRECTORIES} /opt/vc/lib)
link_directories(/usr/local/lib)

add_subdirectory(src)

