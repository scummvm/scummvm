add_library(hdb STATIC)

target_sources(
  hdb
  PRIVATE ai-bots.cpp
          ai-cinematic.cpp
          ai-funcs.cpp
          ai-init.cpp
          ai-inventory.cpp
          ai-lists.cpp
          ai-player.cpp
          ai-use.cpp
          ai-waypoint.cpp
          file-manager.cpp
          gfx.cpp
          hdb.cpp
          input.cpp
          lua-script.cpp
          map.cpp
          menu.cpp
          metaengine.cpp
          sound.cpp
          saveload.cpp
          window.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          ai-player.h
          file-manager.h
          gfx.h
          hdb.h
          input.h
          lua-script.h
          map.h
          menu.h
          sound.h
          window.h
)

add_library(hdb_detection)

target_sources(hdb_detection PRIVATE detection.cpp)
