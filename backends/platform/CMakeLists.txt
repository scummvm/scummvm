cmake_minimum_required(VERSION 3.23)

project(ScummVM)

if (CMAKE_SYSTEM_NAME STREQUAL "Linux")
  add_executable(platform)
endif()

if(PLATFORM_WINDOWS)
  add_executable(platform WIN32)
  target_sources(platform PRIVATE
	sdl/win32/win32-window.cpp
	sdl/win32/win32_wrapper.cpp
	sdl/win32/win32.cpp
  sdl/win32/win32-main.cpp)
endif()

if(PLATFORM_MACOS)
  add_executable(platform MACOSX_BUNDLE)
  target_sources(platform PRIVATE
  sdl/macosx/macosx-main.cpp
	sdl/macosx/macosx.cpp
	sdl/macosx/macosx-touchbar.mm
	sdl/macosx/macosx-window.mm
	sdl/macosx/macosx_wrapper.mm
	sdl/macosx/macosx_osys_misc.mm
	sdl/macosx/appmenu_osx.mm)
endif()

if(PLATFORM_POSIX)
  target_sources(platform PRIVATE sdl/posix/posix.cpp sdl/posix/posix-main.cpp)
endif()

target_link_libraries(platform PRIVATE common backends base)

target_sources(platform PRIVATE sdl/sdl.cpp sdl/sdl-window.cpp)