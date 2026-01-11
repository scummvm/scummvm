cmake_minimum_required(VERSION 3.23) # Current version in Debian

project(ScummVM)
include(FetchContent)

include(cmake/Options.cmake)
include(cmake/Components.cmake)
include(cmake/Engines.cmake)
include(cmake/Dependencies.cmake)

if(NOT DETECT_HOST)
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-multichar")
    set(PLATFORM_POSIX TRUE)
    add_compile_definitions(POSIX)
  endif()

  if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PLATFORM_WINDOWS TRUE CACHE INTERNAL "" FORCE)
    set(WIN32 TRUE CACHE INTERNAL "" FORCE)
    add_compile_definitions(WIN32)
  endif()

  if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(PLATFORM_MACOS TRUE)
    set(PLATFORM_POSIX TRUE)
    add_compile_definitions(MACOSX POSIX)
    link_libraries(
      "-framework AudioUnit -framework AudioToolbox -framework Carbon -framework CoreMIDI -framework Cocoa"
    )
  endif()
endif()

# Wii platform for testing with devkitPPC
if(PLATFORM_WII)
  add_compile_definitions(SCUMM_BIG_ENDIAN)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-multichar")
  add_compile_definitions(__WII__)
endif()

# Needed so we can include using relative paths
include_directories(${CMAKE_SOURCE_DIR})

# This library is a shared definition interface that everything should link to.
add_library(common_libs INTERFACE)

add_subdirectory(common)

add_subdirectory(video)

add_subdirectory(gui)

add_subdirectory(audio)

add_subdirectory(graphics)

add_subdirectory(image)

add_subdirectory(math)

# The main (base) directory
add_subdirectory(base)

add_subdirectory(backends)

# The engines directory
add_subdirectory(engines)

target_link_libraries(
  common_libs
  INTERFACE graphics
            common
            video
            gui
            audio
            image
            math
            base
            backends
            engines
)
