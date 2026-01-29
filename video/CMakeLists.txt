add_library(video)

target_sources(
  video
  PRIVATE 3do_decoder.cpp
          avi_decoder.cpp
          coktel_decoder.cpp
          dxa_decoder.cpp
          flic_decoder.cpp
          mpegps_decoder.cpp
          mve_decoder.cpp
          paco_decoder.cpp
          psx_decoder.cpp
          qt_decoder.cpp
          qtvr_decoder.cpp
          smk_decoder.cpp
          subtitles.cpp
          video_decoder.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          3do_decoder.h
          avi_decoder.h
          coktel_decoder.h
          dxa_decoder.h
          flic_decoder.h
          mpegps_decoder.h
          mve_decoder.h
          paco_decoder.h
          psx_decoder.h
          qt_decoder.h
          smk_decoder.h
          subtitles.h
          video_decoder.h
)
if(USE_BINK)
  target_sources(
    video
    PRIVATE bink_decoder.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            bink_decoder.h
  )
endif()

if(USE_HNM)
  target_sources(
    video
    PRIVATE hnm_decoder.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            hnm_decoder.h
  )
endif()

if(USE_THEORADEC)
  target_sources(
    video
    PRIVATE theora_decoder.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            theora_decoder.h
  )
endif()

if(USE_VPX)
  target_sources(
    video
    PRIVATE mkv_decoder.cpp mkv/mkvparser.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            mkv_decoder.h
            mkv/mkvparser.h
  )
endif()
