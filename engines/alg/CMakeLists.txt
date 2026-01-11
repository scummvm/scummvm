add_library(alg STATIC)

target_sources(
  alg
  PRIVATE alg.cpp
          graphics.cpp
          game.cpp
          metaengine.cpp
          scene.cpp
          video.cpp
          logic/game_bountyhunter.cpp
          logic/game_crimepatrol.cpp
          logic/game_drugwars.cpp
          logic/game_johnnyrock.cpp
          logic/game_maddog.cpp
          logic/game_maddog2.cpp
          logic/game_spacepirates.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          alg.h
          graphics.h
          game.h
          scene.h
          video.h
          logic/game_bountyhunter.h
          logic/game_crimepatrol.h
          logic/game_drugwars.h
          logic/game_johnnyrock.h
          logic/game_maddog.h
          logic/game_maddog2.h
          logic/game_spacepirates.h
)

add_library(alg_detection)

target_sources(alg_detection PRIVATE detection.cpp)
