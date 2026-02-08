# Consolidated OpenDingux toolkit configuration

set(HOST_OS "linux" CACHE INTERNAL "")
set(HOST_CPU "mipsel" CACHE INTERNAL "")

add_compile_definitions(DINGUX OPENDINGUX REDUCE_MEMORY_USAGE UNCACHED_PLUGINS)

if(NOT OD_TARGET AND HOST MATCHES "opendingux-(.+)")
  set(OD_TARGET ${CMAKE_MATCH_1})
endif()

if(OD_TARGET STREQUAL "lepus")
  add_compile_definitions(LEPUS)
  scummvm_toolkit_set_option(USE_HIGHRES OFF)
  scummvm_toolkit_set_option(USE_HQ_SCALERS OFF)
elseif(OD_TARGET STREQUAL "rs90")
  add_compile_definitions(RS90 DISABLE_FANCY_THEMES)
  scummvm_toolkit_set_option(USE_RGB_COLOR OFF)
  scummvm_toolkit_set_option(USE_HIGHRES OFF)
  scummvm_toolkit_set_option(USE_SCALERS OFF)
endif()

# Platform-specific option overrides
scummvm_toolkit_set_option(USE_VKEYBD ON)
scummvm_toolkit_set_option(USE_ALSA OFF)
scummvm_toolkit_set_option(USE_VORBIS OFF)
scummvm_toolkit_set_option(USE_MT32EMU OFF)
scummvm_toolkit_set_option(USE_TIMIDITY OFF)
scummvm_toolkit_set_option(USE_FLUIDSYNTH OFF)
scummvm_toolkit_set_option(USE_THEORADEC OFF)
scummvm_toolkit_set_option(USE_SEQ_MIDI OFF)
scummvm_toolkit_set_option(DISABLE_NUKED_OPL ON)
scummvm_toolkit_set_option(USE_CURL OFF)

set(OD_FLAGS "-fdata-sections -ffunction-sections -mplt")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OD_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} ${OD_FLAGS} -O3 -Wl,--as-needed,--gc-sections"
)

set(DETECT_HOST TRUE CACHE INTERNAL "")
