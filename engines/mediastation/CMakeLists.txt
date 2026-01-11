add_library(mediastation STATIC)

target_sources(
  mediastation
  PRIVATE actor.cpp
          actors/canvas.cpp
          actors/document.cpp
          actors/font.cpp
          actors/hotspot.cpp
          actors/image.cpp
          actors/movie.cpp
          actors/palette.cpp
          actors/path.cpp
          actors/screen.cpp
          actors/sound.cpp
          actors/sprite.cpp
          actors/stage.cpp
          actors/text.cpp
          actors/timer.cpp
          audio.cpp
          bitmap.cpp
          boot.cpp
          clients.cpp
          context.cpp
          cursors.cpp
          datafile.cpp
          graphics.cpp
          mediascript/codechunk.cpp
          mediascript/collection.cpp
          mediascript/eventhandler.cpp
          mediascript/function.cpp
          mediascript/scriptconstants.cpp
          mediascript/scriptvalue.cpp
          mediastation.cpp
          metaengine.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          actor.h
          actors/canvas.h
          actors/document.h
          actors/font.h
          actors/hotspot.h
          actors/image.h
          actors/movie.h
          actors/palette.h
          actors/path.h
          actors/screen.h
          actors/sound.h
          actors/sprite.h
          actors/stage.h
          actors/text.h
          actors/timer.h
          audio.h
          bitmap.h
          boot.h
          clients.h
          context.h
          cursors.h
          datafile.h
          graphics.h
          mediascript/codechunk.h
          mediascript/collection.h
          mediascript/eventhandler.h
          mediascript/function.h
          mediascript/scriptconstants.h
          mediascript/scriptvalue.h
          mediastation.h
          metaengine.h
)

add_library(mediastation_detection)

target_sources(mediastation_detection PRIVATE detection.cpp)
