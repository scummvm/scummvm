add_library(plumbers STATIC)

target_sources(
  plumbers
  PRIVATE plumbers.cpp
          console.cpp
          metaengine.cpp
          windows.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          plumbers.h
          console.h
)

if(USE_RGB_COLOR)
  target_sources(plumbers PRIVATE 3do.cpp)

endif()

add_library(plumbers_detection)

target_sources(plumbers_detection PRIVATE detection.cpp)
