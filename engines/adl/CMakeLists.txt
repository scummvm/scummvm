add_library(adl STATIC)

target_sources(
  adl
  PRIVATE adl.cpp
          adl_v2.cpp
          adl_v3.cpp
          adl_v4.cpp
          adl_v5.cpp
          console.cpp
          disk.cpp
          display.cpp
          display_a2.cpp
          hires1.cpp
          hires2.cpp
          hires4.cpp
          hires5.cpp
          hires6.cpp
          metaengine.cpp
          sound.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          adl.h
          adl_v2.h
          adl_v3.h
          adl_v4.h
          adl_v5.h
          console.h
          disk.h
          display.h
          display_a2.h
          sound.h
)

add_library(adl_detection)

target_sources(adl_detection PRIVATE detection.cpp)

# TODO: Remove this when we build this as a DYNAMIC library
target_sources(adl_detection PRIVATE disk.cpp)
