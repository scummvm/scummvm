add_library(cruise STATIC)

target_sources(
  cruise
  PRIVATE actor.cpp
          background.cpp
          backgroundIncrust.cpp
          cell.cpp
          cruise.cpp
          cruise_main.cpp
          ctp.cpp
          dataLoader.cpp
          debugger.cpp
          decompiler.cpp
          delphine-unpack.cpp
          font.cpp
          function.cpp
          gfxModule.cpp
          linker.cpp
          mainDraw.cpp
          menu.cpp
          metaengine.cpp
          mouse.cpp
          object.cpp
          overlay.cpp
          perso.cpp
          polys.cpp
          saveload.cpp
          script.cpp
          sound.cpp
          stack.cpp
          staticres.cpp
          various.cpp
          vars.cpp
          volume.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          actor.h
          background.h
          backgroundIncrust.h
          cell.h
          cruise.h
          cruise_main.h
          ctp.h
          dataLoader.h
          debugger.h
          font.h
          function.h
          gfxModule.h
          linker.h
          mainDraw.h
          menu.h
          mouse.h
          object.h
          overlay.h
          perso.h
          polys.h
          saveload.h
          script.h
          sound.h
          stack.h
          staticres.h
          various.h
          vars.h
          volume.h
)

add_library(cruise_detection)

target_sources(cruise_detection PRIVATE detection.cpp)
