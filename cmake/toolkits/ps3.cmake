# PS3 toolkit configuration

if(NOT PS3DEV OR NOT PSL1GHT)
  message(FATAL_ERROR "Please set PS3DEV and PSL1GHT in your environment.")
endif()

set(HOST_OS "ps3" CACHE INTERNAL "")
set(HOST_CPU "powerpc" CACHE INTERNAL "")

add_compile_definitions(PLAYSTATION3 SCUMM_BIG_ENDIAN)
set(BACKEND "ps3" CACHE STRING "Backend to build" FORCE)

# Platform-specific option overrides
scummvm_toolkit_set_option(USE_TIMIDITY OFF)
scummvm_toolkit_set_option(USE_VKEYBD ON)

set(PS3_FLAGS "-mcpu=cell -mno-fp-in-toc")
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} ${PS3_FLAGS} -I${PSL1GHT}/ppu/include -I${PS3DEV}/portlibs/ppu/include"
)
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -L${PSL1GHT}/ppu/lib -L${PS3DEV}/portlibs/ppu/lib"
)

set(DETECT_HOST TRUE CACHE INTERNAL "")
