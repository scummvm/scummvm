add_library(macventure STATIC)

target_sources(
  macventure
  PRIVATE container.cpp
          controls.cpp
          cursor.cpp
          datafiles.cpp
          dialog.cpp
          gui.cpp
          image.cpp
          macventure.cpp
          metaengine.cpp
          prebuilt_dialogs.cpp
          saveload.cpp
          script.cpp
          sound.cpp
          text.cpp
          windows.cpp
          world.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          container.h
          controls.h
          dialog.h
          gui.h
          image.h
          macventure.h
          prebuilt_dialogs.h
          script.h
          sound.h
          text.h
          windows.h
          world.h
)

add_library(macventure_detection)

target_sources(macventure_detection PRIVATE detection.cpp)
