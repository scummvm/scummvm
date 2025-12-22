add_library(image)

target_sources(
  image
  PRIVATE ani.cpp
          bmp.cpp
          cel_3do.cpp
          cicn.cpp
          icocur.cpp
          iff.cpp
          jpeg.cpp
          neo.cpp
          pcx.cpp
          pict.cpp
          png.cpp
          scr.cpp
          tga.cpp
          xbm.cpp
          codecs/bmp_raw.cpp
          codecs/cdtoons.cpp
          codecs/cinepak.cpp
          codecs/codec.cpp
          codecs/dither.cpp
          codecs/hlz.cpp
          codecs/jyv1.cpp
          codecs/mjpeg.cpp
          codecs/msrle.cpp
          codecs/msrle4.cpp
          codecs/msvideo1.cpp
          codecs/qtrle.cpp
          codecs/rpza.cpp
          codecs/smc.cpp
          codecs/svq1.cpp
          codecs/truemotion1.cpp
          codecs/xan.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          ani.h
          bmp.h
          cel_3do.h
          cicn.h
          icocur.h
          iff.h
          jpeg.h
          neo.h
          pcx.h
          pict.h
          png.h
          scr.h
          tga.h
          xbm.h
          codecs/bmp_raw.h
          codecs/cdtoons.h
          codecs/cinepak.h
          codecs/codec.h
          codecs/dither.h
          codecs/hlz.h
          codecs/jyv1.h
          codecs/mjpeg.h
          codecs/msrle.h
          codecs/msrle4.h
          codecs/msvideo1.h
          codecs/qtrle.h
          codecs/rpza.h
          codecs/smc.h
          codecs/svq1.h
          codecs/truemotion1.h
          codecs/xan.h)
if(USE_GIF)
  target_sources(
    image
    PRIVATE gif.cpp
    PRIVATE FILE_SET HEADERS FILES gif.h)
endif()

if(USE_MPEG2)
  target_sources(
    image
    PRIVATE codecs/mpeg.cpp
    PRIVATE FILE_SET HEADERS FILES codecs/mpeg.h)
endif()

if(USE_INDEO3)
  target_sources(
    image
    PRIVATE codecs/indeo3.cpp
    PRIVATE FILE_SET HEADERS FILES codecs/indeo3.h)
endif()

if(USE_INDEO45)
  target_sources(
    image
    PRIVATE codecs/indeo4.cpp codecs/indeo5.cpp codecs/indeo/indeo.cpp
            codecs/indeo/indeo_dsp.cpp codecs/indeo/mem.cpp codecs/indeo/vlc.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            codecs/indeo4.h
            codecs/indeo5.h
            codecs/indeo/indeo.h
            codecs/indeo/indeo_dsp.h
            codecs/indeo/mem.h
            codecs/indeo/vlc.h)
endif()

if(USE_HNM)
  target_sources(
    image
    PRIVATE codecs/hnm.cpp
    PRIVATE FILE_SET HEADERS FILES codecs/hnm.h)
endif()
