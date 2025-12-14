cmake_minimum_required(VERSION 3.23)

project(ScummVM)

add_executable(platform)

target_link_libraries(platform PRIVATE common backends base)

target_sources(platform PRIVATE sdl/sdl.cpp sdl/sdl-window.cpp)
if(POSIX)
  target_sources(platform PRIVATE sdl/posix/posix.cpp sdl/posix/posix-main.cpp)
endif()
