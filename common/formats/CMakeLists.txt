cmake_minimum_required(VERSION 3.23)

project(ScummVM)

target_sources(
  common
  PRIVATE cue.cpp
          disk_image.cpp
          formatinfo.cpp
          iff_container.cpp
          ini-file.cpp
          json.cpp
          markdown.cpp
          prodos.cpp
          quicktime.cpp
          winexe.cpp
          winexe_ne.cpp
          winexe_pe.cpp
          xmlparser.cpp)
