cmake_minimum_required(VERSION 3.23)

project(ScummVM)

target_sources(
  common
  PRIVATE clickteam.cpp
          dcl.cpp
          gentee_installer.cpp
          gzio.cpp
          installshield_cab.cpp
          installshieldv3_archive.cpp
          powerpacker.cpp
          rnc_deco.cpp
          stuffit.cpp
          unarj.cpp
          unzip.cpp
          vise.cpp)
