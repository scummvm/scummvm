add_library(gnap STATIC)

target_sources(
  gnap
  PRIVATE character.cpp
          datarchive.cpp
          debugger.cpp
          gamesys.cpp
          gnap.cpp
          grid.cpp
          menu.cpp
          metaengine.cpp
          music.cpp
          resource.cpp
          sound.cpp
          scenes/arcade.cpp
          scenes/groupcs.cpp
          scenes/group0.cpp
          scenes/group1.cpp
          scenes/group2.cpp
          scenes/group3.cpp
          scenes/group4.cpp
          scenes/group5.cpp
          scenes/intro.cpp
          scenes/scenecore.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          character.h
          datarchive.h
          debugger.h
          gamesys.h
          gnap.h
          music.h
          resource.h
          sound.h
          scenes/arcade.h
          scenes/groupcs.h
          scenes/group0.h
          scenes/group1.h
          scenes/group2.h
          scenes/group3.h
          scenes/group4.h
          scenes/group5.h
          scenes/intro.h
          scenes/scenecore.h
)

add_library(gnap_detection)

target_sources(gnap_detection PRIVATE detection.cpp)
