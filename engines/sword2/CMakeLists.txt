add_library(sword2 STATIC)

target_sources(
  sword2
  PRIVATE animation.cpp
          anims.cpp
          console.cpp
          controls.cpp
          debug.cpp
          events.cpp
          function.cpp
          header.cpp
          icons.cpp
          interpreter.cpp
          layers.cpp
          logic.cpp
          maketext.cpp
          memory.cpp
          menu.cpp
          metaengine.cpp
          mouse.cpp
          music.cpp
          palette.cpp
          protocol.cpp
          render.cpp
          resman.cpp
          router.cpp
          saveload.cpp
          screen.cpp
          scroll.cpp
          sound.cpp
          speech.cpp
          sprite.cpp
          startup.cpp
          sword2.cpp
          sync.cpp
          walker.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          animation.h
          console.h
          controls.h
          debug.h
          header.h
          interpreter.h
          logic.h
          maketext.h
          memory.h
          mouse.h
          resman.h
          router.h
          saveload.h
          screen.h
          sound.h
          sword2.h
)

add_library(sword2_detection)

target_sources(sword2_detection PRIVATE detection.cpp)
