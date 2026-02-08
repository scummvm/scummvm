# Nintendo 64 toolkit configuration
set(CMAKE_EXECUTABLE_SUFFIX ".elf" PARENT_SCOPE)
add_compile_definitions(HOSTEXEEXT=".elf")

if(NOT N64SDK)
  message(FATAL_ERROR "Please set N64SDK in your environment.")
endif()

set(HOST_OS "n64" CACHE INTERNAL "")
set(HOST_CPU "mips" CACHE INTERNAL "")

add_compile_definitions(
  __N64__
  LIMIT_FPS
  NONSTANDARD_PORT
  DISABLE_COMMAND_LINE
  DISABLE_DEFAULT_SAVEFILEMANAGER
  DISABLE_DOSBOX_OPL
  DISABLE_FANCY_THEMES
  DISABLE_NES_APU
  REDUCE_MEMORY_USAGE
)

# Platform-specific option overrides
scummvm_toolkit_set_option(USE_MT32EMU OFF)
scummvm_toolkit_set_option(USE_SCALERS OFF)
scummvm_toolkit_set_option(USE_ASPECT OFF)
scummvm_toolkit_set_option(USE_SAVEGAME_TIMESTAMP OFF)
scummvm_toolkit_set_option(USE_TRANSLATION OFF)
scummvm_toolkit_set_option(USE_TEXT_CONSOLE_FOR_DEBUGGER OFF)
scummvm_toolkit_set_option(USE_VKEYBD ON)
scummvm_toolkit_set_option(ENABLE_PLUGINS OFF)
scummvm_toolkit_set_option(USE_MAD OFF)
scummvm_toolkit_set_option(USE_TREMOR ON)
scummvm_toolkit_set_option(USE_ZLIB ON)

set(N64_FLAGS
    "-mno-extern-sdata --param max-inline-insns-auto=20 -fomit-frame-pointer -march=vr4300 -mtune=vr4300 -mhard-float"
)
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} ${N64_FLAGS} -I${N64SDK}/include -I${N64SDK}/mips64/include -I${N64SDK}/hkz-libn64"
)
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} ${N64_FLAGS} -nodefaultlibs -nostartfiles -mno-crt0 -L${N64SDK}/hkz-libn64 -L${N64SDK}/lib -T n64ld_cpp.x"
)

link_libraries(
  pakfs
  framfs
  n64
  n64utils
  romfs
)

set(DETECT_HOST TRUE CACHE INTERNAL "")
