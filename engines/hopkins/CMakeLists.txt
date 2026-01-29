add_library(hopkins STATIC)

target_sources(
  hopkins
  PRIVATE anim.cpp
          computer.cpp
          debugger.cpp
          dialogs.cpp
          events.cpp
          files.cpp
          font.cpp
          graphics.cpp
          globals.cpp
          hopkins.cpp
          lines.cpp
          menu.cpp
          metaengine.cpp
          objects.cpp
          saveload.cpp
          script.cpp
          sound.cpp
          talk.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          anim.h
          computer.h
          debugger.h
          dialogs.h
          events.h
          files.h
          font.h
          graphics.h
          globals.h
          hopkins.h
          lines.h
          menu.h
          objects.h
          saveload.h
          script.h
          sound.h
          talk.h
)

add_library(hopkins_detection)

target_sources(hopkins_detection PRIVATE detection.cpp)
