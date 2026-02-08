# Dreamcast toolkit configuration

if(NOT RONINDIR)
  message(FATAL_ERROR "Please set RONINDIR in your environment. export RONINDIR=<path to libronin>")
endif()

set(HOST_OS "dreamcast" CACHE INTERNAL "")
set(HOST_CPU "sh" CACHE INTERNAL "")

add_compile_definitions(__DC__ NONSTANDARD_PORT)
set(BACKEND "dc" CACHE STRING "Backend to build" FORCE)

# Platform-specific option overrides
scummvm_toolkit_set_option(DISABLE_DEFAULT_SAVEFILEMANAGER ON)
scummvm_toolkit_set_option(DISABLE_TEXT_CONSOLE ON)
scummvm_toolkit_set_option(DISABLE_COMMAND_LINE ON)
scummvm_toolkit_set_option(USE_SCALERS OFF)
scummvm_toolkit_set_option(USE_ASPECT OFF)
scummvm_toolkit_set_option(USE_TINYGL OFF)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_compile_definitions(NOSERIAL)
endif()

set(DC_FLAGS "-ml -m4-single-only")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${DC_FLAGS} -isystem ${RONINDIR}/include")
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} ${DC_FLAGS} -L${RONINDIR}/lib -Wl,-Ttext,0x8c010000 -nostartfiles ${RONINDIR}/lib/crt0.o"
)

link_libraries(ronin-noserial m)

set(DETECT_HOST TRUE CACHE INTERNAL "")
