# Raspberry Pi toolkit configuration

if(NOT RPI_ROOT)
  message(FATAL_ERROR "Please set RPI_ROOT in your environment.")
endif()

set(HOST_OS "linux" CACHE INTERNAL "")
set(HOST_CPU "arm" CACHE INTERNAL "")

set(RPI_LDFLAGS
    "--sysroot=${RPI_ROOT} -B${RPI_ROOT}/usr/lib/arm-linux-gnueabihf -Xlinker --rpath-link=${RPI_ROOT}/usr/lib/arm-linux-gnueabihf/pulseaudio -Xlinker --rpath-link=${RPI_ROOT}/usr/lib/arm-linux-gnueabihf -Xlinker --rpath-link=${RPI_ROOT}/lib/arm-linux-gnueabihf"
)

# Platform-specific option overrides Raspberry Pi usually prefers SDL2 as per configure logic
scummvm_toolkit_set_option(USE_SDL2 ON)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${RPI_LDFLAGS}")
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -isystem ${RPI_ROOT}/usr/include/arm-linux-gnueabihf -I${RPI_ROOT}/usr/include"
)

set(DETECT_HOST TRUE CACHE INTERNAL "")
