target_sources(
  common
  PRIVATE clickteam.cpp
          dcl.cpp
          gentee_installer.cpp
          gzio.cpp
          installshield_cab.cpp
          installshieldv3_archive.cpp
          powerpacker.cpp
          rnc_deco.cpp
          stuffit.cpp
          unarj.cpp
          unzip.cpp
          vise.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          clickteam.h
          dcl.h
          gentee_installer.h
          installshield_cab.h
          installshieldv3_archive.h
          powerpacker.h
          rnc_deco.h
          stuffit.h
          unarj.h
          unzip.h
          vise.h
)
if(USE_ZLIB)
  target_sources(common_compression PRIVATE zlib.cpp)
endif()
