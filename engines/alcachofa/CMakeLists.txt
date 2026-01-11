add_library(alcachofa STATIC)

target_sources(
  alcachofa
  PRIVATE alcachofa.cpp
          camera.cpp
          common.cpp
          console.cpp
          game.cpp
          game-movie-adventure.cpp
          game-objects.cpp
          general-objects.cpp
          global-ui.cpp
          graphics.cpp
          graphics-opengl-base.cpp
          input.cpp
          menu.cpp
          metaengine.cpp
          player.cpp
          rooms.cpp
          scheduler.cpp
          script.cpp
          shape.cpp
          sounds.cpp
          ui-objects.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          alcachofa.h
          camera.h
          common.h
          console.h
          game.h
          global-ui.h
          graphics.h
          graphics-opengl-base.h
          input.h
          menu.h
          metaengine.h
          player.h
          rooms.h
          scheduler.h
          script.h
          shape.h
          sounds.h
)

if(USE_OPENGL_GAME)
  target_sources(
    alcachofa
    PRIVATE graphics-opengl.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            graphics-opengl.h
  )

  if(USE_OPENGL_SHADERS)
    target_sources(
      alcachofa
      PRIVATE graphics-opengl.cpp
      PRIVATE FILE_SET
              HEADERS
              FILES
              graphics-opengl.h
    )

  endif()

endif()

if(USE_OPENGL_GAME)
  target_sources(alcachofa PRIVATE graphics-opengl-classic.cpp)

endif()

if(USE_OPENGL_SHADERS)
  target_sources(alcachofa PRIVATE graphics-opengl-shaders.cpp)

endif()

if(USE_TINYGL)
  target_sources(alcachofa PRIVATE graphics-tinygl.cpp)

endif()

add_library(alcachofa_detection)

target_sources(alcachofa_detection PRIVATE detection.cpp)
