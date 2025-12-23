add_library(drascula STATIC)

target_sources(
  drascula
  PRIVATE actors.cpp
          animation.cpp
          console.cpp
          converse.cpp
          drascula.cpp
          graphics.cpp
          interface.cpp
          metaengine.cpp
          objects.cpp
          palette.cpp
          resource.cpp
          rooms.cpp
          saveload.cpp
          sound.cpp
          talk.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          console.h
          drascula.h
)

add_library(drascula_detection)

target_sources(drascula_detection PRIVATE detection.cpp)
