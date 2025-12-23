add_library(agi STATIC)

target_sources(
  agi
  PRIVATE agi.cpp
          checks.cpp
          console.cpp
          cycle.cpp
          disk_image.cpp
          font.cpp
          global.cpp
          graphics.cpp
          inv.cpp
          keyboard.cpp
          loader.cpp
          loader_a2.cpp
          loader_gal.cpp
          loader_gal_a2.cpp
          loader_v1.cpp
          loader_v2.cpp
          loader_v3.cpp
          logic.cpp
          lzw.cpp
          menu.cpp
          metaengine.cpp
          motion.cpp
          objects.cpp
          opcodes.cpp
          op_cmd.cpp
          op_dbg.cpp
          op_test.cpp
          picture.cpp
          picture_gal.cpp
          saveload.cpp
          sound.cpp
          sound_2gs.cpp
          sound_a2.cpp
          sound_coco3.cpp
          sound_midi.cpp
          sound_pcjr.cpp
          sound_sarien.cpp
          sprite.cpp
          systemui.cpp
          text.cpp
          view.cpp
          words.cpp
          preagi/preagi.cpp
          preagi/mickey.cpp
          preagi/picture_mickey_winnie.cpp
          preagi/picture_troll.cpp
          preagi/troll.cpp
          preagi/winnie.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          agi.h
          console.h
          disk_image.h
          font.h
          graphics.h
          inv.h
          keyboard.h
          loader.h
          logic.h
          lzw.h
          menu.h
          opcodes.h
          picture.h
          picture_gal.h
          sound.h
          sound_2gs.h
          sound_a2.h
          sound_coco3.h
          sound_midi.h
          sound_pcjr.h
          sound_sarien.h
          sprite.h
          systemui.h
          text.h
          view.h
          words.h
          preagi/preagi.h
          preagi/mickey.h
          preagi/picture_mickey_winnie.h
          preagi/picture_troll.h
          preagi/troll.h
          preagi/winnie.h
)

add_library(agi_detection)

target_sources(agi_detection PRIVATE detection.cpp wagparser.cpp)

# TODO: Remove this when we build this as a DYNAMIC library
target_sources(agi_detection PRIVATE disk_image.cpp)
