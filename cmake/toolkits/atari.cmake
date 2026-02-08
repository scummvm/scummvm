# Atari (m68k) toolkit configuration

set(HOST_OS "mint" CACHE INTERNAL "")
set(HOST_CPU "m68k" CACHE INTERNAL "")

set(BACKEND "atari" CACHE STRING "Backend to build" FORCE)

add_compile_definitions(ATARI DISABLE_NES_APU)

# Platform-specific option overrides
scummvm_toolkit_set_option(USE_SDL_NET OFF)
scummvm_toolkit_set_option(USE_CURL OFF)
scummvm_toolkit_set_option(USE_ENET OFF)
scummvm_toolkit_set_option(USE_FLAC OFF)
scummvm_toolkit_set_option(USE_ALSA OFF)
scummvm_toolkit_set_option(USE_SEQ_MIDI OFF)
scummvm_toolkit_set_option(USE_SNDIO OFF)
scummvm_toolkit_set_option(USE_TIMIDITY OFF)
scummvm_toolkit_set_option(USE_MPEG2 OFF)
scummvm_toolkit_set_option(USE_A52 OFF)
scummvm_toolkit_set_option(USE_VPX OFF)
scummvm_toolkit_set_option(USE_FAAD OFF)
scummvm_toolkit_set_option(USE_FLUIDSYNTH OFF)
scummvm_toolkit_set_option(USE_FLUIDLITE OFF)
scummvm_toolkit_set_option(USE_SONIVOX OFF)
scummvm_toolkit_set_option(USE_TINYGL OFF)
scummvm_toolkit_set_option(USE_FRIBIDI OFF)
scummvm_toolkit_set_option(USE_DISCORD OFF)
scummvm_toolkit_set_option(USE_TEXT_CONSOLE_FOR_DEBUGGER ON)
scummvm_toolkit_set_option(USE_MT32EMU OFF)
scummvm_toolkit_set_option(USE_LUA OFF)
scummvm_toolkit_set_option(USE_HQ_SCALERS OFF)
scummvm_toolkit_set_option(USE_CLOUD OFF)
scummvm_toolkit_set_option(USE_TRANSLATION OFF)
scummvm_toolkit_set_option(USE_SID_AUDIO OFF)
scummvm_toolkit_set_option(DISABLE_NUKED_OPL ON)
scummvm_toolkit_set_option(DETECTION_FULL OFF)

set(ATARI_FLAGS "-m68000")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ATARI_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} ${ATARI_FLAGS} -Wl,--msuper-memory -Wl,--stack,256k"
)

link_libraries(gem)

set(DETECT_HOST TRUE CACHE INTERNAL "")
