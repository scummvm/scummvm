add_library(lab STATIC)

target_sources(
  lab
  PRIVATE anim.cpp
          console.cpp
          dispman.cpp
          engine.cpp
          eventman.cpp
          image.cpp
          interface.cpp
          intro.cpp
          lab.cpp
          labsets.cpp
          map.cpp
          metaengine.cpp
          music.cpp
          processroom.cpp
          resource.cpp
          savegame.cpp
          special.cpp
          speciallocks.cpp
          utils.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          anim.h
          console.h
          dispman.h
          eventman.h
          image.h
          interface.h
          intro.h
          lab.h
          labsets.h
          music.h
          processroom.h
          resource.h
          speciallocks.h
          utils.h
)

add_library(lab_detection)

target_sources(lab_detection PRIVATE detection.cpp)
