cmake_minimum_required(VERSION 3.23)

project(ScummVM)

add_library(backends)

target_sources(
  backends
  PRIVATE base-backend.cpp
          modular-backend.cpp
          audiocd/audiocd-stream.cpp
          audiocd/default/default-audiocd.cpp
          events/default/default-events.cpp
          fs/abstract-fs.cpp
          fs/stdiostream.cpp
          keymapper/action.cpp
          keymapper/hardware-input.cpp
          keymapper/input-watcher.cpp
          keymapper/keymap.cpp
          keymapper/keymapper.cpp
          keymapper/remap-widget.cpp
          keymapper/standard-actions.cpp
          keymapper/virtual-mouse.cpp
          log/log.cpp
          midi/alsa.cpp
          midi/dmedia.cpp
          midi/seq.cpp
          midi/sndio.cpp
          midi/stmidi.cpp
          midi/timidity.cpp
          saves/savefile.cpp
          saves/default/default-saves.cpp
          timer/default/default-timer.cpp)

add_subdirectory(platform)

if(PLATFORM_POSIX)
  target_sources(
    backends
    PRIVATE fs/posix/posix-fs.cpp
            fs/posix/posix-fs-factory.cpp
            fs/posix/posix-iostream.cpp
            fs/posix-drives/posix-drives-fs.cpp
            fs/posix-drives/posix-drives-fs-factory.cpp
            fs/chroot/chroot-fs-factory.cpp
            fs/chroot/chroot-fs.cpp
            plugins/posix/posix-provider.cpp
            saves/posix/posix-saves.cpp
            taskbar/unity/unity-taskbar.cpp
            dialogs/gtk/gtk-dialogs.cpp)
endif()

if(PLATFORM_WINDOWS)
  target_sources(
    backends
    PRIVATE audiocd/win32/win32-audiocd.cpp
            dialogs/win32/win32-dialogs.cpp
            fs/windows/windows-fs.cpp
            fs/windows/windows-fs-factory.cpp
            midi/windows.cpp
            plugins/win32/win32-provider.cpp
            saves/windows/windows-saves.cpp
            updates/win32/win32-updates.cpp
            taskbar/win32/win32-taskbar.cpp)
endif()

if(PLATFORM_MACOS)
  target_sources(
    backends
    PRIVATE audiocd/macosx/macosx-audiocd.cpp dialogs/macosx/macosx-dialogs.mm
            midi/coreaudio.cpp midi/coremidi.cpp
            updates/macosx/macosx-updates.mm taskbar/macosx/macosx-taskbar.mm)
endif()

if(SDL_BACKEND)
  target_sources(
    backends
    PRIVATE events/sdl/sdl-common-events.cpp
            graphics/sdl/sdl-graphics.cpp
            graphics/surfacesdl/surfacesdl-graphics.cpp
            mixer/sdl/sdl-mixer.cpp
            mixer/null/null-mixer.cpp
            mutex/sdl/sdl-mutex.cpp
            timer/sdl/sdl-timer.cpp)
endif()

if(USE_SDL2)
  target_sources(backends PRIVATE events/sdl/sdl2-events.cpp)
endif()

target_include_directories(
  backends
  PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/.."
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>)
