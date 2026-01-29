add_library(cge2 STATIC)

target_sources(
  cge2
  PRIVATE cge2.cpp
          vga13h.cpp
          bitmap.cpp
          fileio.cpp
          sound.cpp
          cge2_main.cpp
          text.cpp
          hero.cpp
          snail.cpp
          spare.cpp
          talk.cpp
          events.cpp
          map.cpp
          metaengine.cpp
          vmenu.cpp
          saveload.cpp
          toolbar.cpp
          inventory.cpp
          console.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          cge2.h
          vga13h.h
          bitmap.h
          fileio.h
          sound.h
          cge2_main.h
          text.h
          hero.h
          snail.h
          spare.h
          talk.h
          events.h
          map.h
          vmenu.h
          console.h
)

add_library(cge2_detection)

target_sources(cge2_detection PRIVATE detection.cpp)

# TODO: Remove this when we build this as a DYNAMIC library
target_sources(cge2_detection PRIVATE fileio.cpp)
