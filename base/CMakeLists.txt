add_library(base)

target_sources(
  base
  PRIVATE test_new_standards.cpp main.cpp commandLine.cpp plugins.cpp
          version.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          main.h
          commandLine.h
          plugins.h
          version.h)
