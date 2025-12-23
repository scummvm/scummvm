add_library(dragons STATIC)

target_sources(
  dragons
  PRIVATE actor.cpp
          actorresource.cpp
          background.cpp
          bag.cpp
          bigfile.cpp
          credits.cpp
          cursor.cpp
          cutscene.cpp
          dragonflg.cpp
          dragonimg.cpp
          dragonini.cpp
          dragonobd.cpp
          dragonrms.cpp
          dragonvar.cpp
          dragons.cpp
          font.cpp
          inventory.cpp
          metaengine.cpp
          midimusicplayer.cpp
          minigame1.cpp
          minigame2.cpp
          minigame3.cpp
          minigame4.cpp
          minigame5.cpp
          saveload.cpp
          scene.cpp
          screen.cpp
          scriptopcodes.cpp
          sequenceopcodes.cpp
          specialopcodes.cpp
          sound.cpp
          talk.cpp
          vabsound.cpp
          strplayer.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          actor.h
          actorresource.h
          background.h
          bag.h
          bigfile.h
          credits.h
          cursor.h
          cutscene.h
          dragonflg.h
          dragonimg.h
          dragonini.h
          dragonobd.h
          dragonrms.h
          dragonvar.h
          dragons.h
          font.h
          inventory.h
          midimusicplayer.h
          minigame1.h
          minigame2.h
          minigame3.h
          minigame4.h
          minigame5.h
          scene.h
          screen.h
          scriptopcodes.h
          sequenceopcodes.h
          specialopcodes.h
          sound.h
          talk.h
          vabsound.h
          strplayer.h
)

add_library(dragons_detection)

target_sources(dragons_detection PRIVATE detection.cpp)
