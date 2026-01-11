add_library(tony STATIC)

target_sources(
  tony
  PRIVATE custom.cpp
          debugger.cpp
          font.cpp
          game.cpp
          gfxcore.cpp
          gfxengine.cpp
          globals.cpp
          input.cpp
          inventory.cpp
          loc.cpp
          metaengine.cpp
          sound.cpp
          tony.cpp
          tonychar.cpp
          utils.cpp
          window.cpp
          mpal/expr.cpp
          mpal/loadmpc.cpp
          mpal/memory.cpp
          mpal/mpal.cpp
          mpal/mpalutils.cpp
          mpal/lzo.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          custom.h
          debugger.h
          font.h
          game.h
          gfxcore.h
          gfxengine.h
          globals.h
          input.h
          inventory.h
          loc.h
          sound.h
          tony.h
          tonychar.h
          utils.h
          window.h
          mpal/expr.h
          mpal/loadmpc.h
          mpal/memory.h
          mpal/mpal.h
          mpal/mpalutils.h
          mpal/lzo.h
)

add_library(tony_detection)

target_sources(tony_detection PRIVATE detection.cpp)
