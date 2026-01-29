add_library(tsage STATIC)

target_sources(
  tsage
  PRIVATE converse.cpp
          core.cpp
          debugger.cpp
          dialogs.cpp
          events.cpp
          globals.cpp
          graphics.cpp
          metaengine.cpp
          resources.cpp
          saveload.cpp
          scenes.cpp
          screen.cpp
          sherlock/sherlock_logo.cpp
          sound.cpp
          staticres.cpp
          tsage.cpp
          user_interface.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          converse.h
          core.h
          debugger.h
          dialogs.h
          events.h
          globals.h
          graphics.h
          resources.h
          saveload.h
          scenes.h
          screen.h
          sherlock/sherlock_logo.h
          sound.h
          staticres.h
          tsage.h
          user_interface.h
)

if(ENABLE_BLUEFORCE)
  target_sources(
    tsage
    PRIVATE blue_force/blueforce_dialogs.cpp
            blue_force/blueforce_logic.cpp
            blue_force/blueforce_scenes0.cpp
            blue_force/blueforce_scenes1.cpp
            blue_force/blueforce_scenes2.cpp
            blue_force/blueforce_scenes3.cpp
            blue_force/blueforce_scenes4.cpp
            blue_force/blueforce_scenes5.cpp
            blue_force/blueforce_scenes6.cpp
            blue_force/blueforce_scenes7.cpp
            blue_force/blueforce_scenes8.cpp
            blue_force/blueforce_scenes9.cpp
            blue_force/blueforce_speakers.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            blue_force/blueforce_dialogs.h
            blue_force/blueforce_logic.h
            blue_force/blueforce_scenes0.h
            blue_force/blueforce_scenes1.h
            blue_force/blueforce_scenes2.h
            blue_force/blueforce_scenes3.h
            blue_force/blueforce_scenes4.h
            blue_force/blueforce_scenes5.h
            blue_force/blueforce_scenes6.h
            blue_force/blueforce_scenes7.h
            blue_force/blueforce_scenes8.h
            blue_force/blueforce_scenes9.h
            blue_force/blueforce_speakers.h
  )

endif()

if(ENABLE_RINGWORLD)
  target_sources(
    tsage
    PRIVATE ringworld/ringworld_demo.cpp
            ringworld/ringworld_dialogs.cpp
            ringworld/ringworld_logic.cpp
            ringworld/ringworld_scenes1.cpp
            ringworld/ringworld_scenes2.cpp
            ringworld/ringworld_scenes3.cpp
            ringworld/ringworld_scenes4.cpp
            ringworld/ringworld_scenes5.cpp
            ringworld/ringworld_scenes6.cpp
            ringworld/ringworld_scenes8.cpp
            ringworld/ringworld_scenes10.cpp
            ringworld/ringworld_speakers.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            ringworld/ringworld_demo.h
            ringworld/ringworld_dialogs.h
            ringworld/ringworld_logic.h
            ringworld/ringworld_scenes1.h
            ringworld/ringworld_scenes2.h
            ringworld/ringworld_scenes3.h
            ringworld/ringworld_scenes4.h
            ringworld/ringworld_scenes5.h
            ringworld/ringworld_scenes6.h
            ringworld/ringworld_scenes8.h
            ringworld/ringworld_scenes10.h
            ringworld/ringworld_speakers.h
  )

endif()

if(ENABLE_RINGWORLD2)
  target_sources(
    tsage
    PRIVATE ringworld2/ringworld2_airduct.cpp
            ringworld2/ringworld2_dialogs.cpp
            ringworld2/ringworld2_logic.cpp
            ringworld2/ringworld2_outpost.cpp
            ringworld2/ringworld2_scenes0.cpp
            ringworld2/ringworld2_scenes1.cpp
            ringworld2/ringworld2_scenes2.cpp
            ringworld2/ringworld2_scenes3.cpp
            ringworld2/ringworld2_speakers.cpp
            ringworld2/ringworld2_vampire.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            ringworld2/ringworld2_airduct.h
            ringworld2/ringworld2_dialogs.h
            ringworld2/ringworld2_logic.h
            ringworld2/ringworld2_outpost.h
            ringworld2/ringworld2_scenes0.h
            ringworld2/ringworld2_scenes1.h
            ringworld2/ringworld2_scenes2.h
            ringworld2/ringworld2_scenes3.h
            ringworld2/ringworld2_speakers.h
            ringworld2/ringworld2_vampire.h
  )

endif()

add_library(tsage_detection)

target_sources(tsage_detection PRIVATE detection.cpp)
