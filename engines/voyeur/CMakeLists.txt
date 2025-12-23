add_library(voyeur STATIC)

target_sources(
  voyeur
  PRIVATE animation.cpp
          data.cpp
          debugger.cpp
          events.cpp
          files.cpp
          files_threads.cpp
          metaengine.cpp
          screen.cpp
          sound.cpp
          staticres.cpp
          voyeur.cpp
          voyeur_game.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          animation.h
          data.h
          debugger.h
          events.h
          files.h
          screen.h
          sound.h
          staticres.h
          voyeur.h
)

add_library(voyeur_detection)

target_sources(voyeur_detection PRIVATE detection.cpp)
