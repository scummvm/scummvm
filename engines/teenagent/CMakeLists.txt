add_library(teenagent STATIC)

target_sources(
  teenagent
  PRIVATE actor.cpp
          animation.cpp
          callbacks.cpp
          console.cpp
          dialog.cpp
          font.cpp
          inventory.cpp
          metaengine.cpp
          music.cpp
          objects.cpp
          pack.cpp
          resources.cpp
          scene.cpp
          segment.cpp
          surface.cpp
          surface_list.cpp
          teenagent.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          actor.h
          animation.h
          console.h
          dialog.h
          font.h
          inventory.h
          music.h
          objects.h
          pack.h
          resources.h
          scene.h
          segment.h
          surface.h
          surface_list.h
          teenagent.h
)

add_library(teenagent_detection)

target_sources(teenagent_detection PRIVATE detection.cpp)
