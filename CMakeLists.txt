cmake_minimum_required(VERSION 3.23)

project(ScummVM)
include(FetchContent)

if(NOT DETECT_HOST)
  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-multichar")
    set(PLATFORM_POSIX TRUE)
    add_compile_definitions(POSIX)
  endif()

  if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(PLATFORM_WINDOWS TRUE)
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

  set(SDL_BACKEND TRUE)
  set(USE_SDL2 TRUE)
endif()

add_compile_definitions(DETECTION_STATIC)

# Wii platform for testing with devkitPPC
if(PLATFORM_WII)
  # add_compile_definitions(SCUMM_BIG_ENDIAN)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-multichar")
  add_compile_definitions(__WII__)
endif()

if(SDL_BACKEND)
  # SDL2
  FetchContent_Declare(
    SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG 6510d6ccbf446e058ae8ed95233f59057312df02 # release-2.32.6
    FIND_PACKAGE_ARGS NAMES SDL2)
  FetchContent_MakeAvailable(SDL2)
  link_libraries(SDL2::SDL2)

  add_compile_definitions(SDL_BACKEND USE_SDL2)
endif()

# The engines directory
add_subdirectory(engines)

add_subdirectory(common)

add_subdirectory(video)

add_subdirectory(gui)

add_subdirectory(audio)

add_subdirectory(graphics)

add_subdirectory(image)

# The main (base) directory
add_subdirectory(base)

add_subdirectory(backends)
