cmake_minimum_required(VERSION 3.23)

add_library(
  video
  3do_decoder.cpp
  avi_decoder.cpp
  coktel_decoder.cpp
  dxa_decoder.cpp
  flic_decoder.cpp
  mpegps_decoder.cpp
  mve_decoder.cpp
  paco_decoder.cpp
  psx_decoder.cpp
  qt_decoder.cpp
  qtvr_decoder.cpp
  smk_decoder.cpp
  subtitles.cpp
  video_decoder.cpp)

target_include_directories(
  video
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/.."
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/../.."
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>)
