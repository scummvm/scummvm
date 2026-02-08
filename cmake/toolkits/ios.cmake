# Consolidated iOS/tvOS toolkit configuration

set(HOST_OS "iphoneos" CACHE INTERNAL "")
add_compile_definitions(IPHONE IPHONE_IOS7)
set(BACKEND "ios7" CACHE STRING "Backend to build" FORCE)

# Platform-specific option overrides
scummvm_toolkit_set_option(USE_SEQ_MIDI OFF)
scummvm_toolkit_set_option(USE_TIMIDITY OFF)

if(NOT IOS_TARGET)
  if(HOST MATCHES "ios7-arm64|tvos")
    set(IOS_TARGET ${HOST})
  else()
    set(IOS_TARGET "ios7")
  endif()
endif()

if(IOS_TARGET STREQUAL "tvos")
  set(HOST_CPU "aarch64" CACHE INTERNAL "")
  set(TVOS_FLAGS "-mtvos-version-min=9 -arch arm64")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TVOS_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${TVOS_FLAGS}")
elseif(IOS_TARGET STREQUAL "ios7-arm64")
  set(HOST_CPU "aarch64" CACHE INTERNAL "")
  set(IOS_FLAGS "-miphoneos-version-min=7.1 -arch arm64")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${IOS_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${IOS_FLAGS}")
else()
  set(HOST_CPU "arm" CACHE INTERNAL "")
  set(IOS_FLAGS "-miphoneos-version-min=7.1 -arch armv7")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${IOS_FLAGS}")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${IOS_FLAGS}")
endif()

set(DETECT_HOST TRUE CACHE INTERNAL "")
