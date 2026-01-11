add_library(avalanche STATIC)

target_sources(
  avalanche
  PRIVATE animation.cpp
          avalanche.cpp
          avalot.cpp
          background.cpp
          closing.cpp
          console.cpp
          graphics.cpp
          dropdown.cpp
          parser.cpp
          dialogs.cpp
          sequence.cpp
          sound.cpp
          timer.cpp
          metaengine.cpp
          nim.cpp
          clock.cpp
          ghostroom.cpp
          help.cpp
          shootemup.cpp
          mainmenu.cpp
          highscore.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          animation.h
          avalanche.h
          avalot.h
          background.h
          closing.h
          console.h
          graphics.h
          dropdown.h
          parser.h
          dialogs.h
          sequence.h
          sound.h
          timer.h
          nim.h
          clock.h
          ghostroom.h
          help.h
          shootemup.h
          mainmenu.h
          highscore.h
)

add_library(avalanche_detection)

target_sources(avalanche_detection PRIVATE detection.cpp)
