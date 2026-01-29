add_library(toon STATIC)

target_sources(
  toon
  PRIVATE anim.cpp
          audio.cpp
          character.cpp
          console.cpp
          conversation.cpp
          drew.cpp
          flux.cpp
          font.cpp
          hotspot.cpp
          metaengine.cpp
          movie.cpp
          path.cpp
          picture.cpp
          resource.cpp
          script.cpp
          script_func.cpp
          state.cpp
          subtitles.cpp
          text.cpp
          tools.cpp
          toon.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          anim.h
          audio.h
          character.h
          console.h
          conversation.h
          drew.h
          flux.h
          font.h
          hotspot.h
          movie.h
          path.h
          picture.h
          resource.h
          script.h
          script_func.h
          state.h
          subtitles.h
          text.h
          tools.h
          toon.h
)

add_library(toon_detection)

target_sources(toon_detection PRIVATE detection.cpp)
