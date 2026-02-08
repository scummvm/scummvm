# Maemo toolkit configuration

set(HOST_OS "maemo" CACHE INTERNAL "")
set(HOST_CPU "arm" CACHE INTERNAL "")

add_compile_definitions(MAEMO)

# Platform-specific option overrides
scummvm_toolkit_set_option(USE_VKEYBD ON)
scummvm_toolkit_set_option(USE_HQ_SCALERS OFF)
scummvm_toolkit_set_option(USE_MT32EMU OFF)
scummvm_toolkit_set_option(USE_ALSA OFF)
scummvm_toolkit_set_option(USE_MAD ON)
scummvm_toolkit_set_option(USE_TREMOR ON)
scummvm_toolkit_set_option(USE_ZLIB ON)

set(MAEMO_FLAGS "-mcpu=arm926ej-s -fomit-frame-pointer")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MAEMO_FLAGS} -I/usr/X11R6/include")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/lib")

set(DETECT_HOST TRUE CACHE INTERNAL "")
