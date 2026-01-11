add_library(wage STATIC)

target_sources(
  wage
  PRIVATE combat.cpp
          debugger.cpp
          design.cpp
          entities.cpp
          gui.cpp
          guiborders.cpp
          metaengine.cpp
          randomhat.cpp
          saveload.cpp
          script.cpp
          sound.cpp
          util.cpp
          wage.cpp
          world.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          debugger.h
          design.h
          entities.h
          gui.h
          randomhat.h
          script.h
          sound.h
          wage.h
          world.h
)

if(USE_IMGUI)
  target_sources(
    wage
    PRIVATE debugtools.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            debugtools.h
  )

endif()

add_library(wage_detection)

target_sources(wage_detection PRIVATE detection.cpp)
