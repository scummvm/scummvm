# MorphOS toolkit configuration

set(HOST_OS "morphos" CACHE INTERNAL "")
set(HOST_CPU "powerpc" CACHE INTERNAL "")

add_compile_definitions(__MORPHOS__ SCUMM_BIG_ENDIAN)
set(BACKEND "morphos" CACHE STRING "Backend to build" FORCE)

set(MORPHOS_FLAGS "-mlongcall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MORPHOS_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MORPHOS_FLAGS}")

set(DETECT_HOST TRUE CACHE INTERNAL "")
