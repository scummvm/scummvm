add_library(awe STATIC)

target_sources(
  awe
  PRIVATE metaengine.cpp
          awe.cpp
          aifc_player.cpp
          bitmap.cpp
          engine.cpp
          graphics_gl.cpp
          graphics_soft.cpp
          pak.cpp
          resource.cpp
          resource_3do.cpp
          resource_nth.cpp
          resource_win31.cpp
          script.cpp
          sfx_player.cpp
          sound.cpp
          static_res.cpp
          system_stub.cpp
          unpack.cpp
          video.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          metaengine.h
          awe.h
          aifc_player.h
          bitmap.h
          engine.h
          pak.h
          resource.h
          resource_3do.h
          resource_nth.h
          resource_win31.h
          script.h
          sfx_player.h
          sound.h
          system_stub.h
          unpack.h
          video.h
)

add_library(awe_detection)

target_sources(awe_detection PRIVATE detection.cpp)
