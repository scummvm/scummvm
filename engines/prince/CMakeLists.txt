add_library(prince STATIC)

target_sources(
  prince
  PRIVATE animation.cpp
          archive.cpp
          cursor.cpp
          debugger.cpp
          decompress.cpp
          draw.cpp
          flags.cpp
          font.cpp
          graphics.cpp
          hero.cpp
          inventory.cpp
          metaengine.cpp
          mhwanh.cpp
          music.cpp
          mob.cpp
          object.cpp
          prince.cpp
          pscr.cpp
          resource.cpp
          saveload.cpp
          script.cpp
          sound.cpp
          variatxt.cpp
          videoplayer.cpp
          walk.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          animation.h
          archive.h
          cursor.h
          debugger.h
          decompress.h
          flags.h
          font.h
          graphics.h
          hero.h
          mhwanh.h
          music.h
          mob.h
          object.h
          prince.h
          pscr.h
          resource.h
          script.h
          variatxt.h
)

add_library(prince_detection)

target_sources(prince_detection PRIVATE detection.cpp)
