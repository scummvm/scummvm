cmake_minimum_required(VERSION 3.23)

add_library(engine_plumbers)

add_library(detection_plumbers detection.cpp)

target_link_libraries(engine_plumbers PUBLIC engines video)

target_sources(engine_plumbers PRIVATE plumbers.cpp console.cpp metaengine.cpp
                                       windows.cpp)

target_include_directories(
  engine_plumbers
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/.."
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../.."
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>)

target_include_directories(
  detection_plumbers
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/.."
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../.."
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>)
