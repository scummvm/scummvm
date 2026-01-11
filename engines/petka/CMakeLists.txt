add_library(petka STATIC)

target_sources(
  petka
  PRIVATE big_dialogue.cpp
          file_mgr.cpp
          flc.cpp
          metaengine.cpp
          petka.cpp
          saveload.cpp
          q_manager.cpp
          q_system.cpp
          sound.cpp
          video.cpp
          walk.cpp
          interfaces/dialog_interface.cpp
          interfaces/interface.cpp
          interfaces/main.cpp
          interfaces/map.cpp
          interfaces/panel.cpp
          interfaces/save_load.cpp
          interfaces/sequence.cpp
          interfaces/startup.cpp
          objects/object.cpp
          objects/object_bg.cpp
          objects/object_case.cpp
          objects/object_cursor.cpp
          objects/object_star.cpp
          objects/heroes.cpp
          objects/text.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          big_dialogue.h
          file_mgr.h
          flc.h
          petka.h
          q_manager.h
          q_system.h
          sound.h
          video.h
          walk.h
          interfaces/dialog_interface.h
          interfaces/interface.h
          interfaces/main.h
          interfaces/map.h
          interfaces/panel.h
          interfaces/save_load.h
          interfaces/sequence.h
          interfaces/startup.h
          objects/object.h
          objects/object_bg.h
          objects/object_case.h
          objects/object_cursor.h
          objects/object_star.h
          objects/heroes.h
          objects/text.h
)

add_library(petka_detection)

target_sources(petka_detection PRIVATE detection.cpp)
