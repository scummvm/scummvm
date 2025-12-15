cmake_minimum_required(VERSION 3.23)

project(ScummVM)
include(FetchContent)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-multichar")

set(POSIX TRUE)
set(SDL_BACKEND TRUE)
set(USE_SDL2 TRUE)


# SDL2
FetchContent_Declare(
  SDL2
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG        6510d6ccbf446e058ae8ed95233f59057312df02 # release-2.32.6
  FIND_PACKAGE_ARGS NAMES SDL2
)
FetchContent_MakeAvailable(SDL2)
link_libraries(SDL2::SDL2)

add_compile_definitions(POSIX SDL_BACKEND USE_SDL2)

add_subdirectory(common)

# The engines directory
add_subdirectory(engines)

add_subdirectory(gui)

add_subdirectory(backends)

add_subdirectory(audio)

add_subdirectory(graphics)

add_subdirectory(image)

# The main (base) directory
add_subdirectory(base)
