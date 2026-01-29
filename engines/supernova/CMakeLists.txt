add_library(supernova STATIC)

target_sources(
  supernova
  PRIVATE console.cpp
          graphics.cpp
          metaengine.cpp
          resman.cpp
          room.cpp
          supernova1/rooms.cpp
          supernova2/rooms.cpp
          screen.cpp
          sound.cpp
          game-manager.cpp
          supernova1/state.cpp
          supernova2/state.cpp
          supernova.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          console.h
          graphics.h
          resman.h
          room.h
          supernova1/rooms.h
          supernova2/rooms.h
          screen.h
          sound.h
          game-manager.h
          supernova1/state.h
          supernova2/state.h
          supernova.h
)

add_library(supernova_detection)

target_sources(supernova_detection PRIVATE detection.cpp)
