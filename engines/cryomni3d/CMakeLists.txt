add_library(cryomni3d STATIC)

target_sources(
  cryomni3d
  PRIVATE fonts/cryoextfont.cpp
          fonts/cryofont.cpp
          image/hlz.cpp
          image/hnm.cpp
          cryomni3d.cpp
          datstream.cpp
          dialogs_manager.cpp
          fixed_image.cpp
          font_manager.cpp
          metaengine.cpp
          mouse_boxes.cpp
          objects.cpp
          omni3d.cpp
          sprites.cpp
          wam_parser.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          fonts/cryoextfont.h
          fonts/cryofont.h
          image/hlz.h
          image/hnm.h
          cryomni3d.h
          datstream.h
          dialogs_manager.h
          fixed_image.h
          font_manager.h
          mouse_boxes.h
          objects.h
          omni3d.h
          sprites.h
          wam_parser.h
)

if(ENABLE_VERSAILLES)
  target_sources(
    cryomni3d
    PRIVATE versailles/data.cpp
            versailles/dialogs_manager.cpp
            versailles/dialogs.cpp
            versailles/documentation.cpp
            versailles/engine.cpp
            versailles/logic.cpp
            versailles/menus.cpp
            versailles/music.cpp
            versailles/saveload.cpp
            versailles/toolbar.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            versailles/dialogs_manager.h
            versailles/documentation.h
            versailles/engine.h
            versailles/toolbar.h
  )

endif()

add_library(cryomni3d_detection)

target_sources(cryomni3d_detection PRIVATE detection.cpp)
