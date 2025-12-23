add_library(testbed STATIC)

target_sources(
  testbed
  PRIVATE config.cpp
          config-params.cpp
          events.cpp
          fs.cpp
          graphics.cpp
          image.cpp
          metaengine.cpp
          midi.cpp
          misc.cpp
          networking.cpp
          savegame.cpp
          sound.cpp
          testbed.cpp
          testsuite.cpp
          video.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          config.h
          config-params.h
          events.h
          fs.h
          graphics.h
          image.h
          midi.h
          misc.h
          networking.h
          savegame.h
          sound.h
          testbed.h
          testsuite.h
          video.h
)

if(USE_CLOUD)
  target_sources(
    testbed
    PRIVATE cloud.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            cloud.h
  )

endif()

if(USE_SDL_NET)
  target_sources(
    testbed
    PRIVATE webserver.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            webserver.h
  )

endif()

if(USE_TTS)
  target_sources(
    testbed
    PRIVATE speech.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            speech.h
  )

endif()

if(USE_IMGUI)
  target_sources(
    testbed
    PRIVATE imgui.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            imgui.h
  )

endif()

if(USE_TINYGL AND USE_OPENGL_GAME)
  target_sources(
    testbed
    PRIVATE tinygl.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            tinygl.h
  )
endif()

add_library(testbed_detection)

target_sources(testbed_detection PRIVATE detection.cpp)
