add_library(dm STATIC)

target_sources(
  dm
  PRIVATE champion.cpp
          console.cpp
          dialog.cpp
          dm.cpp
          dmglobals.cpp
          dungeonman.cpp
          eventman.cpp
          gfx.cpp
          group.cpp
          inventory.cpp
          loadsave.cpp
          lzw.cpp
          menus.cpp
          metaengine.cpp
          movesens.cpp
          objectman.cpp
          projexpl.cpp
          sounds.cpp
          text.cpp
          timeline.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          champion.h
          console.h
          dialog.h
          dm.h
          dungeonman.h
          eventman.h
          gfx.h
          group.h
          inventory.h
          loadsave.h
          lzw.h
          menus.h
          movesens.h
          objectman.h
          projexpl.h
          sounds.h
          text.h
          timeline.h
)

add_library(dm_detection)

target_sources(dm_detection PRIVATE detection.cpp)
