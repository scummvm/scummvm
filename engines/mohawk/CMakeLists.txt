add_library(mohawk STATIC)

target_sources(
  mohawk
  PRIVATE bitmap.cpp
          console.cpp
          cursors.cpp
          dialogs.cpp
          graphics.cpp
          installer_archive.cpp
          livingbooks.cpp
          livingbooks_code.cpp
          livingbooks_graphics.cpp
          livingbooks_lbx.cpp
          metaengine.cpp
          riven_metaengine.cpp
          myst_metaengine.cpp
          mohawk.cpp
          resource.cpp
          sound.cpp
          video.cpp
          view.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          bitmap.h
          console.h
          cursors.h
          dialogs.h
          graphics.h
          installer_archive.h
          livingbooks.h
          livingbooks_code.h
          livingbooks_graphics.h
          livingbooks_lbx.h
          riven_metaengine.h
          myst_metaengine.h
          mohawk.h
          resource.h
          sound.h
          video.h
          view.h
)

if(ENABLE_CSTIME)
  target_sources(
    mohawk
    PRIVATE cstime.cpp
            cstime_cases.cpp
            cstime_game.cpp
            cstime_graphics.cpp
            cstime_ui.cpp
            cstime_view.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            cstime.h
            cstime_cases.h
            cstime_game.h
            cstime_graphics.h
            cstime_ui.h
            cstime_view.h
  )

endif()

if(ENABLE_MYST)
  target_sources(
    mohawk
    PRIVATE myst.cpp
            myst_areas.cpp
            myst_card.cpp
            myst_graphics.cpp
            myst_scripts.cpp
            myst_sound.cpp
            myst_state.cpp
            resource_cache.cpp
            myst_stacks/channelwood.cpp
            myst_stacks/credits.cpp
            myst_stacks/demo.cpp
            myst_stacks/dni.cpp
            myst_stacks/intro.cpp
            myst_stacks/makingof.cpp
            myst_stacks/mechanical.cpp
            myst_stacks/menu.cpp
            myst_stacks/myst.cpp
            myst_stacks/preview.cpp
            myst_stacks/selenitic.cpp
            myst_stacks/slides.cpp
            myst_stacks/stoneship.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            myst.h
            myst_areas.h
            myst_card.h
            myst_graphics.h
            myst_scripts.h
            myst_sound.h
            myst_state.h
            resource_cache.h
            myst_stacks/channelwood.h
            myst_stacks/credits.h
            myst_stacks/demo.h
            myst_stacks/dni.h
            myst_stacks/intro.h
            myst_stacks/makingof.h
            myst_stacks/mechanical.h
            myst_stacks/menu.h
            myst_stacks/myst.h
            myst_stacks/preview.h
            myst_stacks/selenitic.h
            myst_stacks/slides.h
            myst_stacks/stoneship.h
  )

endif()

if(ENABLE_RIVEN)
  target_sources(
    mohawk
    PRIVATE riven.cpp
            riven_card.cpp
            riven_graphics.cpp
            riven_inventory.cpp
            riven_saveload.cpp
            riven_scripts.cpp
            riven_sound.cpp
            riven_stack.cpp
            riven_vars.cpp
            riven_video.cpp
            riven_stacks/aspit.cpp
            riven_stacks/bspit.cpp
            riven_stacks/domespit.cpp
            riven_stacks/gspit.cpp
            riven_stacks/jspit.cpp
            riven_stacks/ospit.cpp
            riven_stacks/pspit.cpp
            riven_stacks/rspit.cpp
            riven_stacks/tspit.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            riven.h
            riven_card.h
            riven_graphics.h
            riven_inventory.h
            riven_saveload.h
            riven_scripts.h
            riven_sound.h
            riven_stack.h
            riven_video.h
            riven_stacks/aspit.h
            riven_stacks/bspit.h
            riven_stacks/domespit.h
            riven_stacks/gspit.h
            riven_stacks/jspit.h
            riven_stacks/ospit.h
            riven_stacks/pspit.h
            riven_stacks/rspit.h
            riven_stacks/tspit.h
  )

endif()

add_library(mohawk_detection)

target_sources(mohawk_detection PRIVATE detection.cpp)

# TODO: Remove this when we build this as a DYNAMIC library
target_sources(mohawk_detection PRIVATE myst_metaengine.cpp riven_metaengine.cpp)
