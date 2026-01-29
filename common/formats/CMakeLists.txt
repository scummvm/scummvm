add_library(common_formats)

target_sources(
  common_formats
  PRIVATE cue.cpp
          disk_image.cpp
          formatinfo.cpp
          iff_container.cpp
          ini-file.cpp
          json.cpp
          markdown.cpp
          prodos.cpp
          quicktime.cpp
          winexe.cpp
          winexe_ne.cpp
          winexe_pe.cpp
          xmlparser.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          cue.h
          disk_image.h
          formatinfo.h
          iff_container.h
          ini-file.h
          json.h
          markdown.h
          prodos.h
          quicktime.h
          winexe.h
          winexe_ne.h
          winexe_pe.h
          xmlparser.h
)
