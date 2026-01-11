add_library(sky STATIC)

target_sources(
  sky
  PRIVATE autoroute.cpp
          compact.cpp
          control.cpp
          debug.cpp
          disk.cpp
          grid.cpp
          hufftext.cpp
          intro.cpp
          logic.cpp
          metaengine.cpp
          mouse.cpp
          screen.cpp
          sky.cpp
          sound.cpp
          text.cpp
          music/adlibchannel.cpp
          music/adlibmusic.cpp
          music/gmchannel.cpp
          music/gmmusic.cpp
          music/mt32music.cpp
          music/musicbase.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          autoroute.h
          compact.h
          control.h
          debug.h
          disk.h
          grid.h
          intro.h
          logic.h
          mouse.h
          screen.h
          sky.h
          sound.h
          text.h
          music/adlibchannel.h
          music/adlibmusic.h
          music/gmchannel.h
          music/gmmusic.h
          music/mt32music.h
          music/musicbase.h
)

add_library(sky_detection)

target_sources(sky_detection PRIVATE detection.cpp)
