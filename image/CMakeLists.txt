cmake_minimum_required(VERSION 3.23)

project(ScummVM)

add_library(image)

target_link_libraries(image PRIVATE common)
target_sources(
  image
  PRIVATE ani.cpp
          bmp.cpp
          cel_3do.cpp
          cicn.cpp
          icocur.cpp
          iff.cpp
          jpeg.cpp
          neo.cpp
          pcx.cpp
          pict.cpp
          png.cpp
          scr.cpp
          tga.cpp
          xbm.cpp
          codecs/bmp_raw.cpp
          codecs/cdtoons.cpp
          codecs/cinepak.cpp
          codecs/codec.cpp
          codecs/dither.cpp
          codecs/hlz.cpp
          codecs/jyv1.cpp
          codecs/mjpeg.cpp
          codecs/msrle.cpp
          codecs/msrle4.cpp
          codecs/msvideo1.cpp
          codecs/qtrle.cpp
          codecs/rpza.cpp
          codecs/smc.cpp
          codecs/svq1.cpp
          codecs/truemotion1.cpp
          codecs/xan.cpp)
