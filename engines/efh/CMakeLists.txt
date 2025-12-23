add_library(efh STATIC)

target_sources(
  efh
  PRIVATE constants.cpp
          efh.cpp
          fight.cpp
          files.cpp
          graphics.cpp
          init.cpp
          menu.cpp
          metaengine.cpp
          savegames.cpp
          script.cpp
          sound.cpp
          utils.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          constants.h
          efh.h
)

add_library(efh_detection)

target_sources(efh_detection PRIVATE detection.cpp)
