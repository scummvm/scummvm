# Consolidated RISC OS toolkit configuration
set(CMAKE_EXECUTABLE_SUFFIX ",e1f" PARENT_SCOPE)
add_compile_definitions(HOSTEXEEXT=",e1f")

if(NOT GCCSDK_INSTALL_ENV)
  message(FATAL_ERROR "Please set GCCSDK_INSTALL_ENV in your environment.")
endif()

set(HOST_OS "riscos" CACHE INTERNAL "")
set(HOST_CPU "arm" CACHE INTERNAL "")
add_compile_definitions(RISCOS)

# Platform-specific option overrides
scummvm_toolkit_set_option(USE_ELF_LOADER ON)

if(NOT RISCOS_ARCH)
  if(HOST STREQUAL "arm-vfp-riscos")
    set(RISCOS_ARCH "vfp")
  else()
    set(RISCOS_ARCH "standard")
  endif()
endif()

if(RISCOS_ARCH STREQUAL "vfp")
  set(RISCOS_PATH "$GCCSDK_INSTALL_ENV/vfp")
  add_compile_options("-march=armv6zk" "-mtune=cortex-a9" "-mfpu=vfp")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mfpu=vfp")
  set(RISCOS_PATH "$GCCSDK_INSTALL_ENV")
  add_compile_options("-march=armv3m" "-mtune=xscale")
  scummvm_toolkit_set_option(USE_VORBIS OFF)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -isystem ${RISCOS_PATH}/include")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L${RISCOS_PATH}/lib -static")

set(DETECT_HOST TRUE CACHE INTERNAL "")
