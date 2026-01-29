add_library(freescape STATIC)

target_sources(
  freescape
  PRIVATE area.cpp
          assets.cpp
          font.cpp
          events.cpp
          demo.cpp
          doodle.cpp
          freescape.cpp
          games/castle/castle.cpp
          games/castle/amiga.cpp
          games/castle/cpc.cpp
          games/castle/dos.cpp
          games/castle/zx.cpp
          games/dark/amiga.cpp
          games/dark/atari.cpp
          games/dark/c64.cpp
          games/dark/cpc.cpp
          games/dark/dark.cpp
          games/dark/dos.cpp
          games/dark/zx.cpp
          games/driller/amiga.cpp
          games/driller/atari.cpp
          games/driller/c64.cpp
          games/driller/c64.music.cpp
          games/driller/cpc.cpp
          games/driller/dos.cpp
          games/driller/driller.cpp
          games/driller/zx.cpp
          games/eclipse/atari.cpp
          games/eclipse/c64.cpp
          games/eclipse/dos.cpp
          games/eclipse/eclipse.cpp
          games/eclipse/cpc.cpp
          games/eclipse/zx.cpp
          games/palettes.cpp
          gfx.cpp
          loaders/8bitImage.cpp
          loaders/8bitBinaryLoader.cpp
          loaders/c64.cpp
          language/8bitDetokeniser.cpp
          language/instruction.cpp
          metaengine.cpp
          movement.cpp
          objects/geometricobject.cpp
          objects/group.cpp
          objects/sensor.cpp
          sweepAABB.cpp
          sound.cpp
          ui.cpp
          unpack.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          area.h
          font.h
          doodle.h
          freescape.h
          games/castle/castle.h
          games/dark/dark.h
          games/driller/c64.music.h
          games/driller/driller.h
          games/eclipse/eclipse.h
          gfx.h
          language/8bitDetokeniser.h
          language/instruction.h
          objects/geometricobject.h
          objects/group.h
          objects/sensor.h
          sweepAABB.h
          sound.h
)

if(USE_TINYGL)
  target_sources(
    freescape
    PRIVATE gfx_tinygl.cpp gfx_tinygl_texture.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            gfx_tinygl.h
            gfx_tinygl_texture.h
  )

endif()

if(USE_OPENGL_SHADERS)
  target_sources(
    freescape
    PRIVATE gfx_opengl_shaders.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            gfx_opengl_shaders.h
  )

endif()

if(USE_OPENGL)
  target_sources(
    freescape
    PRIVATE gfx_opengl.cpp gfx_opengl_texture.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            gfx_opengl.h
            gfx_opengl_texture.h
  )

endif()

add_library(freescape_detection)

target_sources(freescape_detection PRIVATE detection.cpp)
