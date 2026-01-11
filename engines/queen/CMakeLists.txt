add_library(queen STATIC)

target_sources(
  queen
  PRIVATE bankman.cpp
          command.cpp
          credits.cpp
          cutaway.cpp
          debug.cpp
          display.cpp
          graphics.cpp
          grid.cpp
          input.cpp
          journal.cpp
          logic.cpp
          metaengine.cpp
          midiadlib.cpp
          music.cpp
          musicdata.cpp
          queen.cpp
          resource.cpp
          restables.cpp
          sound.cpp
          state.cpp
          talk.cpp
          version.cpp
          walk.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          bankman.h
          command.h
          credits.h
          cutaway.h
          debug.h
          display.h
          graphics.h
          grid.h
          input.h
          journal.h
          logic.h
          midiadlib.h
          music.h
          queen.h
          resource.h
          sound.h
          state.h
          talk.h
          version.h
          walk.h
)

add_library(queen_detection)

target_sources(queen_detection PRIVATE detection.cpp)

# TODO: Remove this when we build this as a DYNAMIC library
target_sources(queen_detection PRIVATE version.cpp)
