add_library(saga STATIC)

target_sources(
  saga
  PRIVATE actor.cpp
          actor_path.cpp
          actor_walk.cpp
          animation.cpp
          console.cpp
          events.cpp
          font.cpp
          font_map.cpp
          gfx.cpp
          image.cpp
          input.cpp
          interface.cpp
          introproc_ite.cpp
          isomap.cpp
          itedata.cpp
          metaengine.cpp
          music.cpp
          objectmap.cpp
          palanim.cpp
          puzzle.cpp
          render.cpp
          resource.cpp
          resource_rsc.cpp
          saga.cpp
          saveload.cpp
          scene.cpp
          script.cpp
          sfuncs.cpp
          sndres.cpp
          sound.cpp
          sprite.cpp
          sthread.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          actor.h
          animation.h
          console.h
          events.h
          font.h
          gfx.h
          interface.h
          isomap.h
          itedata.h
          music.h
          objectmap.h
          palanim.h
          puzzle.h
          render.h
          resource.h
          saga.h
          scene.h
          script.h
          sndres.h
          sound.h
          sprite.h
)

if(ENABLE_IHNM)
  target_sources(saga PRIVATE introproc_ihnm.cpp resource_res.cpp sfuncs_ihnm.cpp)

endif()

add_library(saga_detection)

target_sources(saga_detection PRIVATE detection.cpp)
