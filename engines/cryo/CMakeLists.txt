add_library(cryo STATIC)

target_sources(
  cryo
  PRIVATE cryo.cpp
          cryolib.cpp
          debugger.cpp
          eden.cpp
          eden_graphics.cpp
          metaengine.cpp
          resource.cpp
          sound.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          cryo.h
          cryolib.h
          debugger.h
          eden.h
          eden_graphics.h
          sound.h
)

add_library(cryo_detection)

target_sources(cryo_detection PRIVATE detection.cpp)
