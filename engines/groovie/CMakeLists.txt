add_library(groovie STATIC)

target_sources(
  groovie
  PRIVATE logic/cell.cpp
          video/player.cpp
          video/vdx.cpp
          cursor.cpp
          debug.cpp
          font.cpp
          graphics.cpp
          groovie.cpp
          lzss.cpp
          metaengine.cpp
          music.cpp
          resource.cpp
          saveload.cpp
          script.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          logic/cell.h
          video/player.h
          video/vdx.h
          cursor.h
          debug.h
          font.h
          graphics.h
          groovie.h
          lzss.h
          music.h
          resource.h
          saveload.h
          script.h
)

if(ENABLE_GROOVIE2)
  target_sources(
    groovie
    PRIVATE logic/beehive.cpp
            logic/cake.cpp
            logic/gallery.cpp
            logic/mousetrap.cpp
            logic/othello.cpp
            logic/pente.cpp
            logic/tlcgame.cpp
            logic/triangle.cpp
            logic/winerack.cpp
            video/roq.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            logic/beehive.h
            logic/cake.h
            logic/gallery.h
            logic/mousetrap.h
            logic/othello.h
            logic/pente.h
            logic/tlcgame.h
            logic/triangle.h
            logic/winerack.h
            video/roq.h
  )

endif()

add_library(groovie_detection)

target_sources(groovie_detection PRIVATE detection.cpp)
