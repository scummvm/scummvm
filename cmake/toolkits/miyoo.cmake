# Consolidated Miyoo toolkit configuration

set(HOST_OS "miyoo" CACHE INTERNAL "")
set(HOST_CPU "arm" CACHE INTERNAL "")

add_compile_definitions(MIYOO REDUCE_MEMORY_USAGE UNCACHED_PLUGINS)

if(NOT MIYOO_TARGET)
  if(HOST STREQUAL "miyoomini")
    set(MIYOO_TARGET "miyoomini")
  else()
    set(MIYOO_TARGET "miyoo")
  endif()
endif()

if(MIYOO_TARGET STREQUAL "miyoomini")
  add_compile_definitions(MIYOOMINI)
  set(MIYOO_FLAGS
      "-marm -mtune=cortex-a7 -mfpu=neon-vfpv4 -mfloat-abi=hard -O3 -march=armv7ve -ffast-math -fomit-frame-pointer -fno-strength-reduce"
  )
  scummvm_toolkit_set_option(USE_HIGHRES ON)
else()
  set(MIYOO_FLAGS
      "-march=armv5te -mtune=arm926ej-s -ffast-math -fomit-frame-pointer -ffunction-sections -fdata-sections"
  )
  scummvm_toolkit_set_option(USE_HIGHRES OFF)
endif()

# Platform-specific option overrides
scummvm_toolkit_set_option(USE_VKEYBD ON)
scummvm_toolkit_set_option(USE_ALSA OFF)
scummvm_toolkit_set_option(USE_MT32EMU OFF)
scummvm_toolkit_set_option(USE_TIMIDITY OFF)
scummvm_toolkit_set_option(USE_FLUIDSYNTH OFF)
scummvm_toolkit_set_option(USE_SEQ_MIDI OFF)
scummvm_toolkit_set_option(DISABLE_NUKED_OPL ON)
scummvm_toolkit_set_option(USE_CURL OFF)
scummvm_toolkit_set_option(USE_VORBIS OFF)
scummvm_toolkit_set_option(USE_TREMOR ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MIYOO_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} ${MIYOO_FLAGS} -O3 -Wl,--as-needed,--gc-sections"
)

set(DETECT_HOST TRUE CACHE INTERNAL "")
