cmake_minimum_required(VERSION 3.5)

project(clipper CXX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

set(CLIPPER_SRC clipper.cpp)
add_library(clipper STATIC ${CLIPPER_SRC})
