# Consolidated Android toolkit configuration

if(NOT ANDROID_SDK_ROOT OR NOT ANDROID_NDK_ROOT)
  message(FATAL_ERROR "Please set ANDROID_SDK_ROOT and ANDROID_NDK_ROOT in your environment.")
endif()

set(HOST_OS "android" CACHE INTERNAL "")

# Handle ABI selection
if(NOT ABI)
  if(HOST MATCHES "android-arm-v7a|android-armeabi-v7a|ouya")
    set(ABI "armeabi-v7a")
  elseif(HOST STREQUAL "android-arm64-v8a")
    set(ABI "arm64-v8a")
  elseif(HOST STREQUAL "android-x86")
    set(ABI "x86")
  elseif(HOST STREQUAL "android-x86_64")
    set(ABI "x86_64")
  else()
    set(ABI "armeabi-v7a") # Default
  endif()
endif()

set(HOST_EXE_PREFIX "lib" PARENT_SCOPE)

# Platform-specific option overrides
scummvm_toolkit_set_option(USE_ASPECT OFF)
scummvm_toolkit_set_option(USE_MIDI OFF)
scummvm_toolkit_set_option(USE_TIMIDITY OFF)
scummvm_toolkit_set_option(USE_CURL OFF)

if(ABI STREQUAL "armeabi-v7a")
  set(HOST_CPU "arm" CACHE INTERNAL "")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mfpu=neon")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--fix-cortex-a8")
  if(HOST STREQUAL "ouya")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mtune=cortex-a9")
  endif()
elseif(ABI STREQUAL "arm64-v8a")
  set(HOST_CPU "aarch64" CACHE INTERNAL "")
elseif(ABI STREQUAL "x86")
  set(HOST_CPU "i686" CACHE INTERNAL "")
elseif(ABI STREQUAL "x86_64")
  set(HOST_CPU "x86_64" CACHE INTERNAL "")
endif()

add_compile_definitions(ANDROID_BACKEND NONSTANDARD_PORT)

set(ANDROID_COMMON_FLAGS "-fpic -ffunction-sections -funwind-tables -Wa,--noexecstack")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ANDROID_COMMON_FLAGS} -Wno-inconsistent-missing-override")
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} -shared -Wl,-Bsymbolic,--no-undefined -static-libstdc++ -Wl,-z,noexecstack -Wl,-z,max-page-size=16384 -Wl,--build-id=sha1"
)

set(DETECT_HOST TRUE CACHE INTERNAL "")
