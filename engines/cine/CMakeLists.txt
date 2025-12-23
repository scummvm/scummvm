add_library(cine STATIC)

target_sources(
  cine
  PRIVATE anim.cpp
          bg.cpp
          bg_list.cpp
          console.cpp
          cine.cpp
          gfx.cpp
          main_loop.cpp
          metaengine.cpp
          msg.cpp
          object.cpp
          pal.cpp
          part.cpp
          prc.cpp
          rel.cpp
          saveload.cpp
          script_fw.cpp
          script_os.cpp
          sound.cpp
          texte.cpp
          unpack.cpp
          various.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          anim.h
          bg.h
          bg_list.h
          console.h
          cine.h
          gfx.h
          main_loop.h
          msg.h
          object.h
          pal.h
          part.h
          prc.h
          rel.h
          saveload.h
          sound.h
          texte.h
          unpack.h
          various.h
)

add_library(cine_detection)

target_sources(cine_detection PRIVATE detection.cpp)
