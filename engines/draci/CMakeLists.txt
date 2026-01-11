add_library(draci STATIC)

target_sources(
  draci
  PRIVATE animation.cpp
          barchive.cpp
          console.cpp
          draci.cpp
          font.cpp
          game.cpp
          metaengine.cpp
          mouse.cpp
          music.cpp
          saveload.cpp
          screen.cpp
          script.cpp
          sound.cpp
          sprite.cpp
          surface.cpp
          walking.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          animation.h
          barchive.h
          console.h
          draci.h
          font.h
          game.h
          mouse.h
          music.h
          saveload.h
          screen.h
          script.h
          sound.h
          sprite.h
          surface.h
          walking.h
)

add_library(draci_detection)

target_sources(draci_detection PRIVATE detection.cpp)
