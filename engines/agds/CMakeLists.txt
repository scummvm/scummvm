add_library(agds STATIC)

target_sources(
  agds
  PRIVATE agds.cpp
          animation.cpp
          character.cpp
          console.cpp
          database.cpp
          dialog.cpp
          font.cpp
          inventory.cpp
          metaengine.cpp
          mjpgPlayer.cpp
          mouseMap.cpp
          object.cpp
          patch.cpp
          process.cpp
          process_opcodes.cpp
          region.cpp
          resourceManager.cpp
          screen.cpp
          soundManager.cpp
          systemVariable.cpp
          textLayout.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          agds.h
          animation.h
          character.h
          console.h
          database.h
          dialog.h
          font.h
          inventory.h
          mjpgPlayer.h
          mouseMap.h
          object.h
          patch.h
          process.h
          region.h
          resourceManager.h
          screen.h
          soundManager.h
          systemVariable.h
          textLayout.h
)

add_library(agds_detection)

target_sources(agds_detection PRIVATE detection.cpp)
