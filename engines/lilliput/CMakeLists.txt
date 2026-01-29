add_library(lilliput STATIC)

target_sources(
  lilliput
  PRIVATE console.cpp
          lilliput.cpp
          metaengine.cpp
          script.cpp
          sound.cpp
          stream.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          console.h
          lilliput.h
          script.h
          sound.h
          stream.h
)

add_library(lilliput_detection)

target_sources(lilliput_detection PRIVATE detection.cpp)
