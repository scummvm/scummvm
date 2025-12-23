add_library(trecision STATIC)

target_sources(
  trecision
  PRIVATE console.cpp
          actor.cpp
          animmanager.cpp
          animtype.cpp
          dialog.cpp
          fastfile.cpp
          graphics.cpp
          inventory.cpp
          logic.cpp
          metaengine.cpp
          pathfinding3d.cpp
          renderer3d.cpp
          resource.cpp
          sound.cpp
          saveload.cpp
          scheduler.cpp
          script.cpp
          struct.cpp
          text.cpp
          trecision.cpp
          utils.cpp
          video.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          console.h
          actor.h
          animmanager.h
          animtype.h
          dialog.h
          fastfile.h
          graphics.h
          logic.h
          pathfinding3d.h
          renderer3d.h
          sound.h
          scheduler.h
          struct.h
          text.h
          trecision.h
          video.h
)

add_library(trecision_detection)

target_sources(trecision_detection PRIVATE detection.cpp)
