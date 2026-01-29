add_library(parallaction STATIC)

target_sources(
  parallaction
  PRIVATE adlib.cpp
          balloons.cpp
          callables_br.cpp
          callables_ns.cpp
          debug.cpp
          dialogue.cpp
          disk_br.cpp
          disk_ns.cpp
          exec.cpp
          exec_br.cpp
          exec_ns.cpp
          font.cpp
          gfxbase.cpp
          graphics.cpp
          gui.cpp
          gui_br.cpp
          gui_ns.cpp
          input.cpp
          inventory.cpp
          metaengine.cpp
          objects.cpp
          parallaction.cpp
          parallaction_br.cpp
          parallaction_ns.cpp
          parser.cpp
          parser_br.cpp
          parser_ns.cpp
          saveload.cpp
          sound_ns.cpp
          sound_br.cpp
          staticres.cpp
          walk.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          debug.h
          exec.h
          graphics.h
          gui.h
          input.h
          inventory.h
          objects.h
          parallaction.h
          parser.h
          saveload.h
          walk.h
)

add_library(parallaction_detection)

target_sources(parallaction_detection PRIVATE detection.cpp)
