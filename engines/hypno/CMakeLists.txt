add_library(hypno STATIC)

target_sources(
  hypno
  PRIVATE actions.cpp
          arcade.cpp
          boyz/arcade.cpp
          boyz/boyz.cpp
          boyz/hard.cpp
          boyz/scene.cpp
          cursors.cpp
          grammar_mis.cpp
          grammar_arc.cpp
          hypno.cpp
          lexer_mis.cpp
          lexer_arc.cpp
          libfile.cpp
          metaengine.cpp
          scene.cpp
          spider/arcade.cpp
          spider/hard.cpp
          spider/spider.cpp
          spider/talk.cpp
          video.cpp
          wet/arcade.cpp
          wet/cursors.cpp
          wet/hard.cpp
          wet/wet.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          hypno.h
          libfile.h
)

# TODO: Add missing logic

add_library(hypno_detection)

target_sources(hypno_detection PRIVATE detection.cpp)
