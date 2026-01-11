add_library(immortal STATIC)

target_sources(
  immortal
  PRIVATE bullet.cpp
          compression.cpp
          cycle.cpp
          door.cpp
          drawChr.cpp
          flameSet.cpp
          immortal.cpp
          kernal.cpp
          level.cpp
          logic.cpp
          metaengine.cpp
          misc.cpp
          room.cpp
          sprites.cpp
          story.cpp
          utilities.cpp
          univ.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          immortal.h
          metaengine.h
          room.h
          story.h
          utilities.h
)

add_library(immortal_detection)

target_sources(immortal_detection PRIVATE detection.cpp)
