add_library(hadesch STATIC)

target_sources(
  hadesch
  PRIVATE metaengine.cpp
          pod_file.cpp
          tag_file.cpp
          pod_image.cpp
          video.cpp
          hadesch.cpp
          baptr.cpp
          rooms/olympus.cpp
          rooms/walloffame.cpp
          rooms/argo.cpp
          rooms/crete.cpp
          rooms/minos.cpp
          rooms/daedalus.cpp
          rooms/seriphos.cpp
          rooms/medisle.cpp
          rooms/troy.cpp
          rooms/quiz.cpp
          rooms/minotaur.cpp
          rooms/catacombs.cpp
          rooms/priam.cpp
          rooms/athena.cpp
          rooms/volcano.cpp
          rooms/riverstyx.cpp
          rooms/hadesthrone.cpp
          rooms/credits.cpp
          rooms/intro.cpp
          rooms/ferry.cpp
          rooms/options.cpp
          rooms/monster.cpp
          rooms/monster/projectile.cpp
          rooms/monster/typhoon.cpp
          rooms/monster/cyclops.cpp
          rooms/monster/illusion.cpp
          rooms/medusa.cpp
          rooms/trojan.cpp
          gfx_context.cpp
          ambient.cpp
          herobelt.cpp
          hotzone.cpp
          table.cpp
          persistent.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          pod_file.h
          tag_file.h
          pod_image.h
          video.h
          hadesch.h
          baptr.h
          rooms/monster.h
          gfx_context.h
          ambient.h
          herobelt.h
          hotzone.h
          table.h
          persistent.h
)

add_library(hadesch_detection)

target_sources(hadesch_detection PRIVATE detection.cpp)

# TODO: Add HADESCH make rule targets
