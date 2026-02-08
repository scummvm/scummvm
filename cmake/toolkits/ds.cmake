# DS toolkit configuration
set(CMAKE_EXECUTABLE_SUFFIX ".elf" PARENT_SCOPE)
add_compile_definitions(HOSTEXEEXT=".elf")

if(NOT DEVKITPRO)
  message(
    FATAL_ERROR "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to devkitPRO>"
  )
endif()

set(HOST_OS "ds" CACHE INTERNAL "")
set(HOST_CPU "arm" CACHE INTERNAL "")

add_compile_definitions(
  __DS__
  __NDS__
  ARM9
  ARM
  NONSTANDARD_PORT
  REDUCE_MEMORY_USAGE
  STREAM_AUDIO_FROM_DISK
  VECTOR_RENDERER_FORMAT=1555
)

# Platform-specific option overrides
scummvm_toolkit_set_option(DISABLE_HELP_STRINGS ON)
scummvm_toolkit_set_option(DISABLE_DOSBOX_OPL ON)
scummvm_toolkit_set_option(DISABLE_FANCY_THEMES ON)
scummvm_toolkit_set_option(DISABLE_NES_APU ON)
scummvm_toolkit_set_option(USE_SCALERS OFF)
scummvm_toolkit_set_option(USE_ASPECT OFF)
scummvm_toolkit_set_option(USE_MT32EMU OFF)
scummvm_toolkit_set_option(DISABLE_NUKED_OPL ON)
scummvm_toolkit_set_option(USE_TINYGL OFF)
scummvm_toolkit_set_option(USE_BINK OFF)
scummvm_toolkit_set_option(USE_LUA OFF)
scummvm_toolkit_set_option(USE_PNG OFF)
scummvm_toolkit_set_option(USE_FREETYPE OFF)

if(ENABLE_PLUGINS)
  scummvm_toolkit_set_option(DETECTION_STATIC OFF)
  scummvm_toolkit_set_option(DEFAULT_DYNAMIC ON)
  add_compile_definitions(UNCACHED_PLUGINS)
else()
  scummvm_toolkit_set_option(DETECTION_FULL OFF)
endif()

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(DISABLE_TEXT_CONSOLE)
endif()

set(DS_CXX_FLAGS
    "-isystem ${DEVKITPRO}/libnds/include -isystem ${DEVKITPRO}/portlibs/nds/include -isystem ${DEVKITPRO}/portlibs/armv5te/include -march=armv5te -mtune=arm946e-s -fomit-frame-pointer -mthumb -mthumb-interwork -ffunction-sections -fdata-sections -fno-strict-aliasing -fno-threadsafe-statics -fuse-cxa-atexit -Wno-format"
)
set(DS_LD_FLAGS
    "-mthumb -mthumb-interwork -mfloat-abi=soft -L${DEVKITPRO}/libnds/lib -L${DEVKITPRO}/portlibs/nds/lib -L${DEVKITPRO}/portlibs/armv5te/lib"
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DS_CXX_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${DS_LD_FLAGS}")

link_libraries(filesystem fat mm9 nds9)

set(DETECT_HOST TRUE CACHE INTERNAL "")
