add_library(myst3 STATIC)

target_sources(
  myst3
  PRIVATE ambient.cpp
          archive.cpp
          console.cpp
          cursor.cpp
          database.cpp
          effects.cpp
          gfx.cpp
          gfx_opengl.cpp
          gfx_opengl_shaders.cpp
          gfx_opengl_texture.cpp
          hotspot.cpp
          inventory.cpp
          menu.cpp
          metaengine.cpp
          movie.cpp
          myst3.cpp
          node.cpp
          nodecube.cpp
          nodeframe.cpp
          puzzles.cpp
          scene.cpp
          script.cpp
          sound.cpp
          state.cpp
          subtitles.cpp
          transition.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          ambient.h
          archive.h
          console.h
          cursor.h
          database.h
          effects.h
          gfx.h
          gfx_opengl.h
          gfx_opengl_shaders.h
          gfx_opengl_texture.h
          hotspot.h
          inventory.h
          menu.h
          movie.h
          myst3.h
          node.h
          nodecube.h
          nodeframe.h
          puzzles.h
          scene.h
          script.h
          sound.h
          state.h
          subtitles.h
          transition.h
)

if(USE_TINYGL)
  target_sources(
    myst3
    PRIVATE gfx_tinygl.cpp gfx_tinygl_texture.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            gfx_tinygl.h
            gfx_tinygl_texture.h
  )

endif()

add_library(myst3_detection)

target_sources(myst3_detection PRIVATE detection.cpp)
