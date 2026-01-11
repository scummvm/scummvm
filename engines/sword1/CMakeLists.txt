add_library(sword1 STATIC)

target_sources(
  sword1
  PRIVATE animation.cpp
          console.cpp
          control.cpp
          debug.cpp
          eventman.cpp
          logic.cpp
          memman.cpp
          menu.cpp
          metaengine.cpp
          mouse.cpp
          objectman.cpp
          resman.cpp
          router.cpp
          screen.cpp
          sound.cpp
          staticres.cpp
          sword1.cpp
          text.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          animation.h
          console.h
          control.h
          debug.h
          eventman.h
          logic.h
          memman.h
          menu.h
          mouse.h
          objectman.h
          resman.h
          router.h
          screen.h
          sound.h
          sword1.h
          text.h
)

add_library(sword1_detection)

target_sources(sword1_detection PRIVATE detection.cpp)
