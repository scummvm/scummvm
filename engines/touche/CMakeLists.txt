add_library(touche STATIC)

target_sources(
  touche
  PRIVATE console.cpp
          graphics.cpp
          menu.cpp
          metaengine.cpp
          midi.cpp
          opcodes.cpp
          resource.cpp
          saveload.cpp
          staticres.cpp
          touche.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          console.h
          graphics.h
          midi.h
          touche.h
)

add_library(touche_detection)

target_sources(touche_detection PRIVATE detection.cpp)
