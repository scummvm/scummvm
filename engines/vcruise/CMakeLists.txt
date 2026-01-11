add_library(vcruise STATIC)

target_sources(
  vcruise
  PRIVATE ad2044_items.cpp
          ad2044_ui.cpp
          audio_player.cpp
          circuitpuzzle.cpp
          midi_player.cpp
          metaengine.cpp
          menu.cpp
          runtime.cpp
          runtime_scriptexec.cpp
          sampleloop.cpp
          script.cpp
          textparser.cpp
          vcruise.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          ad2044_items.h
          ad2044_ui.h
          audio_player.h
          circuitpuzzle.h
          midi_player.h
          menu.h
          runtime.h
          sampleloop.h
          script.h
          textparser.h
          vcruise.h
)

add_library(vcruise_detection)

target_sources(vcruise_detection PRIVATE detection.cpp)
