add_library(lure STATIC)

target_sources(
  lure
  PRIVATE animseq.cpp
          debugger.cpp
          decode.cpp
          disk.cpp
          events.cpp
          fights.cpp
          game.cpp
          hotspots.cpp
          intro.cpp
          lure.cpp
          memory.cpp
          menu.cpp
          metaengine.cpp
          palette.cpp
          res.cpp
          res_struct.cpp
          room.cpp
          screen.cpp
          scripts.cpp
          sound.cpp
          strings.cpp
          surface.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          animseq.h
          debugger.h
          decode.h
          disk.h
          events.h
          fights.h
          game.h
          hotspots.h
          intro.h
          lure.h
          memory.h
          menu.h
          palette.h
          res.h
          res_struct.h
          room.h
          screen.h
          scripts.h
          sound.h
          strings.h
          surface.h
)

add_library(lure_detection)

target_sources(lure_detection PRIVATE detection.cpp)
