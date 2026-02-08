# AmigaOS toolkit configuration

set(HOST_OS "amigaos" CACHE INTERNAL "")
set(HOST_CPU "powerpc" CACHE INTERNAL "")

add_compile_definitions(__amigaos4__ SCUMM_BIG_ENDIAN)
set(BACKEND "amigaos" CACHE STRING "Backend to build" FORCE)

set(AMIGAOS_FLAGS "-mlongcall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${AMIGAOS_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${AMIGAOS_FLAGS}")

set(DETECT_HOST TRUE CACHE INTERNAL "")
