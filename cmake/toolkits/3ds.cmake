# 3DS toolkit configuration
set(CMAKE_EXECUTABLE_SUFFIX ".elf" PARENT_SCOPE)
add_compile_definitions(HOSTEXEEXT=".elf")

if(NOT DEVKITPRO)
  message(
    FATAL_ERROR "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to devkitPRO>"
  )
endif()

set(HOST_OS "3ds" CACHE INTERNAL "")
set(HOST_CPU "arm" CACHE INTERNAL "")

add_compile_definitions(__3DS__ ARM ARM11)

# Platform-specific option overrides
scummvm_toolkit_set_option(DISABLE_FANCY_THEMES ON)
scummvm_toolkit_set_option(DISABLE_HELP_STRINGS ON)
scummvm_toolkit_set_option(DISABLE_NES_APU ON)
scummvm_toolkit_set_option(USE_SCALERS OFF)
scummvm_toolkit_set_option(USE_VKEYBD ON)
scummvm_toolkit_set_option(USE_MIDI OFF)
scummvm_toolkit_set_option(USE_MT32EMU OFF)
scummvm_toolkit_set_option(USE_TIMIDITY OFF)
scummvm_toolkit_set_option(USE_VORBIS OFF)
scummvm_toolkit_set_option(USE_TREMOR ON)
scummvm_toolkit_set_option(USE_ELF_LOADER ON)

if(ENABLE_PLUGINS)
  scummvm_toolkit_set_option(DETECTION_STATIC OFF)
  scummvm_toolkit_set_option(DEFAULT_DYNAMIC ON)
  add_compile_definitions(UNCACHED_PLUGINS)
else()
  scummvm_toolkit_set_option(DETECTION_FULL OFF)
endif()

set(3DS_FLAGS
    "-march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft -mword-relocations -ffunction-sections -fomit-frame-pointer"
)
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} ${3DS_FLAGS} -I${DEVKITPRO}/libctru/include -I${DEVKITPRO}/portlibs/3ds/include"
)
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} ${3DS_FLAGS} -L${DEVKITPRO}/libctru/lib -L${DEVKITPRO}/portlibs/3ds/lib -specs=3dsx.specs -lctru"
)

link_libraries(citro3d ctru)

set(DETECT_HOST TRUE CACHE INTERNAL "")
