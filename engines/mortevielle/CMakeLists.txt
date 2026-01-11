add_library(mortevielle STATIC)

target_sources(
  mortevielle
  PRIVATE actions.cpp
          debugger.cpp
          dialogs.cpp
          graphics.cpp
          menu.cpp
          metaengine.cpp
          mortevielle.cpp
          mouse.cpp
          outtext.cpp
          saveload.cpp
          sound.cpp
          utils.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          debugger.h
          dialogs.h
          graphics.h
          menu.h
          mortevielle.h
          mouse.h
          outtext.h
          saveload.h
          sound.h
)

add_library(mortevielle_detection)

target_sources(mortevielle_detection PRIVATE detection.cpp)
