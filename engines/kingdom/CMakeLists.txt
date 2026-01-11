add_library(kingdom STATIC)

target_sources(
  kingdom
  PRIVATE kingdom.cpp
          constants.cpp
          console.cpp
          logic1.cpp
          logic2.cpp
          logic3.cpp
          logic4.cpp
          logic.cpp
          metaengine.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          kingdom.h
          console.h
          logic.h
)

add_library(kingdom_detection)

target_sources(kingdom_detection PRIVATE detection.cpp)
