add_library(cge STATIC)

target_sources(
  cge
  PRIVATE bitmap.cpp
          cge.cpp
          cge_main.cpp
          console.cpp
          events.cpp
          fileio.cpp
          game.cpp
          metaengine.cpp
          snail.cpp
          sound.cpp
          talk.cpp
          text.cpp
          vga13h.cpp
          vmenu.cpp
          walk.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          bitmap.h
          cge.h
          cge_main.h
          console.h
          events.h
          fileio.h
          game.h
          snail.h
          sound.h
          talk.h
          text.h
          vga13h.h
          vmenu.h
          walk.h
)

add_library(cge_detection)

target_sources(cge_detection PRIVATE detection.cpp)

# TODO: Remove this when we build this as a DYNAMIC library
target_sources(cge_detection PRIVATE fileio.cpp)
