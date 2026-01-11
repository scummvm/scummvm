add_library(griffon STATIC)

target_sources(
  griffon
  PRIVATE combat.cpp
          console.cpp
          cutscenes.cpp
          dialogs.cpp
          draw.cpp
          engine.cpp
          gfx.cpp
          griffon.cpp
          input.cpp
          logic.cpp
          metaengine.cpp
          resources.cpp
          saveload.cpp
          sound.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          console.h
          griffon.h
)

add_library(griffon_detection)

target_sources(griffon_detection PRIVATE detection.cpp)
