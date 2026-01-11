add_library(tucker STATIC)

target_sources(
  tucker
  PRIVATE console.cpp
          graphics.cpp
          locations.cpp
          metaengine.cpp
          resource.cpp
          saveload.cpp
          sequences.cpp
          staticres.cpp
          tucker.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          console.h
          graphics.h
          tucker.h
)

add_library(tucker_detection)

target_sources(tucker_detection PRIVATE detection.cpp)
