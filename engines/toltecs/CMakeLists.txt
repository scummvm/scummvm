add_library(toltecs STATIC)

target_sources(
  toltecs
  PRIVATE animation.cpp
          console.cpp
          menu.cpp
          metaengine.cpp
          microtiles.cpp
          movie.cpp
          music.cpp
          palette.cpp
          toltecs.cpp
          render.cpp
          resource.cpp
          saveload.cpp
          screen.cpp
          script.cpp
          segmap.cpp
          sound.cpp
          sprite.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          animation.h
          console.h
          menu.h
          microtiles.h
          movie.h
          music.h
          palette.h
          toltecs.h
          render.h
          resource.h
          screen.h
          script.h
          segmap.h
          sound.h
)

add_library(toltecs_detection)

target_sources(toltecs_detection PRIVATE detection.cpp)
