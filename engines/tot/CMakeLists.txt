add_library(tot STATIC)

target_sources(
  tot
  PRIVATE anims.cpp
          chrono.cpp
          console.cpp
          cutscenes.cpp
          debug.cpp
          decoder/TotFlicDecoder.cpp
          dialog.cpp
          engine.cpp
          events.cpp
          forest.cpp
          graphics.cpp
          metaengine.cpp
          midi.cpp
          mouse.cpp
          resources.cpp
          saveload.cpp
          sound.cpp
          tot.cpp
          util.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          anims.h
          chrono.h
          console.h
          debug.h
          decoder/TotFlicDecoder.h
          dialog.h
          events.h
          forest.h
          graphics.h
          metaengine.h
          midi.h
          mouse.h
          sound.h
          tot.h
          util.h
)

add_library(tot_detection)

target_sources(tot_detection PRIVATE detection.cpp)
