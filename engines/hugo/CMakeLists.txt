add_library(hugo STATIC)

target_sources(
  hugo
  PRIVATE console.cpp
          dialogs.cpp
          display.cpp
          file.cpp
          file_v1d.cpp
          file_v2d.cpp
          file_v3d.cpp
          file_v1w.cpp
          file_v2w.cpp
          hugo.cpp
          intro.cpp
          inventory.cpp
          metaengine.cpp
          mouse.cpp
          object.cpp
          object_v1d.cpp
          object_v1w.cpp
          object_v2d.cpp
          object_v3d.cpp
          parser.cpp
          parser_v1w.cpp
          parser_v1d.cpp
          parser_v2d.cpp
          parser_v3d.cpp
          route.cpp
          schedule.cpp
          sound.cpp
          text.cpp
          util.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          console.h
          dialogs.h
          display.h
          file.h
          hugo.h
          intro.h
          inventory.h
          mouse.h
          object.h
          parser.h
          route.h
          schedule.h
          sound.h
          text.h
          util.h
)

add_library(hugo_detection)

target_sources(hugo_detection PRIVATE detection.cpp)
