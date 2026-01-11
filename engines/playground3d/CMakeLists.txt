add_library(playground3d STATIC)

target_sources(
  playground3d
  PRIVATE metaengine.cpp
          gfx.cpp
          gfx_opengl.cpp
          gfx_opengl_shaders.cpp
          playground3d.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          gfx.h
          gfx_opengl.h
          gfx_opengl_shaders.h
          playground3d.h
)

if(USE_TINYGL)
  target_sources(
    playground3d
    PRIVATE gfx_tinygl.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            gfx_tinygl.h
  )

endif()

add_library(playground3d_detection)

target_sources(playground3d_detection PRIVATE detection.cpp)
