add_library(bbvs STATIC)

target_sources(
  bbvs
  PRIVATE bbvs.cpp
          dialogs.cpp
          gamemodule.cpp
          graphics.cpp
          logic.cpp
          metaengine.cpp
          saveload.cpp
          scene.cpp
          sound.cpp
          spritemodule.cpp
          videoplayer.cpp
          walk.cpp
          minigames/bbairguitar.cpp
          minigames/bbairguitar_anims.cpp
          minigames/bbant.cpp
          minigames/bbant_anims.cpp
          minigames/bbloogie.cpp
          minigames/bbloogie_anims.cpp
          minigames/bbtennis.cpp
          minigames/bbtennis_anims.cpp
          minigames/minigame.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          bbvs.h
          dialogs.h
          gamemodule.h
          graphics.h
          sound.h
          spritemodule.h
          minigames/bbairguitar.h
          minigames/bbant.h
          minigames/bbloogie.h
          minigames/bbtennis.h
          minigames/minigame.h
)

add_library(bbvs_detection)

target_sources(bbvs_detection PRIVATE detection.cpp)
