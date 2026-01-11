add_library(dreamweb STATIC)

target_sources(
  dreamweb
  PRIVATE backdrop.cpp
          console.cpp
          dreamweb.cpp
          keypad.cpp
          metaengine.cpp
          monitor.cpp
          mouse.cpp
          newplace.cpp
          object.cpp
          pathfind.cpp
          people.cpp
          print.cpp
          rain.cpp
          rnca_archive.cpp
          saveload.cpp
          sound.cpp
          sprite.cpp
          stubs.cpp
          talk.cpp
          titles.cpp
          use.cpp
          vgafades.cpp
          vgagrafx.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          console.h
          dreamweb.h
          rnca_archive.h
          sound.h
)

add_library(dreamweb_detection)

target_sources(dreamweb_detection PRIVATE detection.cpp)
