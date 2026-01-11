add_library(chamber STATIC)

target_sources(
  chamber
  PRIVATE anim.cpp
          bkbuff.cpp
          cga.cpp
          chamber.cpp
          cursor.cpp
          decompr.cpp
          dialog.cpp
          ifgm.cpp
          input.cpp
          invent.cpp
          kult.cpp
          menu.cpp
          metaengine.cpp
          portrait.cpp
          print.cpp
          r_pers.cpp
          r_puzzl.cpp
          r_sprit.cpp
          r_texts.cpp
          resdata.cpp
          room.cpp
          savegame.cpp
          script.cpp
          sound.cpp
          timer.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          anim.h
          cga.h
          chamber.h
          cursor.h
          decompr.h
          dialog.h
          ifgm.h
          input.h
          invent.h
          menu.h
          portrait.h
          print.h
          resdata.h
          room.h
          savegame.h
          script.h
          sound.h
          timer.h
)

add_library(chamber_detection)

target_sources(chamber_detection PRIVATE detection.cpp)
