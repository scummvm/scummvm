cmake_minimum_required(VERSION 3.23)

project(ScummVM)

if(POSIX)
  add_executable(platform)
  target_sources(platform PRIVATE sdl/posix/posix.cpp sdl/posix/posix-main.cpp)
endif()

if(PLATFORM_WINDOWS)
  add_executable(platform WIN32)
  target_sources(platform PRIVATE
	sdl/win32/win32-window.cpp
	sdl/win32/win32_wrapper.cpp
	sdl/win32/win32.cpp
  sdl/win32/win32-main.cpp)
endif()

target_link_libraries(platform PRIVATE common backends base)

target_sources(platform PRIVATE sdl/sdl.cpp sdl/sdl-window.cpp)