add_library(made STATIC)

target_sources(
  made
  PRIVATE console.cpp
          database.cpp
          graphics.cpp
          made.cpp
          metaengine.cpp
          music.cpp
          pmvplayer.cpp
          redreader.cpp
          resource.cpp
          screen.cpp
          screenfx.cpp
          script.cpp
          scriptfuncs.cpp
          sound.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          console.h
          database.h
          graphics.h
          made.h
          music.h
          pmvplayer.h
          redreader.h
          resource.h
          screen.h
          screenfx.h
          script.h
          scriptfuncs.h
          sound.h
)

add_library(made_detection)

target_sources(made_detection PRIVATE detection.cpp)
