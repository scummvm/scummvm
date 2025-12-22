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
          timer/default/default-timer.cpp
  PRIVATE FILE_SET
          HEADERS
          FILES
          base-backend.h
          modular-backend.h
          audiocd/audiocd-stream.h
          audiocd/default/default-audiocd.h
          events/default/default-events.h
          fs/abstract-fs.h
          fs/stdiostream.h
          keymapper/action.h
          keymapper/hardware-input.h
          keymapper/input-watcher.h
          keymapper/keymap.h
          keymapper/keymapper.h
          keymapper/remap-widget.h
          keymapper/standard-actions.h
          keymapper/virtual-mouse.h
          log/log.h
          saves/default/default-saves.h
          timer/default/default-timer.h)

add_subdirectory(platform)

if(USE_HTTP)
  target_sources(
    backends
    PRIVATE networking/http/connectionmanager.cpp
            networking/http/networkreadstream.cpp
            networking/http/httpjsonrequest.cpp
            networking/http/httprequest.cpp
            networking/http/postrequest.cpp
            networking/http/request.cpp
            networking/http/session.cpp
            networking/http/sessionrequest.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            networking/http/connectionmanager.h
            networking/http/networkreadstream.h
            networking/http/httpjsonrequest.h
            networking/http/httprequest.h
            networking/http/postrequest.h
            networking/http/request.h
            networking/http/session.h
            networking/http/sessionrequest.h)
endif()

if(USE_CLOUD)
  target_sources(
    backends
    PRIVATE cloud/basestorage.cpp
            cloud/cloudicon.cpp
            cloud/cloudmanager.cpp
            cloud/iso8601.cpp
            cloud/storage.cpp
            cloud/storagefile.cpp
            cloud/downloadrequest.cpp
            cloud/folderdownloadrequest.cpp
            cloud/savessyncrequest.cpp
            cloud/box/boxstorage.cpp
            cloud/box/boxlistdirectorybyidrequest.cpp
            cloud/box/boxtokenrefresher.cpp
            cloud/box/boxuploadrequest.cpp
            cloud/dropbox/dropboxstorage.cpp
            cloud/dropbox/dropboxcreatedirectoryrequest.cpp
            cloud/dropbox/dropboxinforequest.cpp
            cloud/dropbox/dropboxlistdirectoryrequest.cpp
            cloud/dropbox/dropboxtokenrefresher.cpp
            cloud/dropbox/dropboxuploadrequest.cpp
            cloud/googledrive/googledrivelistdirectorybyidrequest.cpp
            cloud/googledrive/googledrivestorage.cpp
            cloud/googledrive/googledrivetokenrefresher.cpp
            cloud/googledrive/googledriveuploadrequest.cpp
            cloud/id/idstorage.cpp
            cloud/id/idcreatedirectoryrequest.cpp
            cloud/id/iddownloadrequest.cpp
            cloud/id/idlistdirectoryrequest.cpp
            cloud/id/idresolveidrequest.cpp
            cloud/id/idstreamfilerequest.cpp
            cloud/onedrive/onedrivestorage.cpp
            cloud/onedrive/onedrivecreatedirectoryrequest.cpp
            cloud/onedrive/onedrivetokenrefresher.cpp
            cloud/onedrive/onedrivelistdirectoryrequest.cpp
            cloud/onedrive/onedriveuploadrequest.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            cloud/basestorage.h
            cloud/cloudicon.h
            cloud/cloudmanager.h
            cloud/iso8601.h
            cloud/storage.h
            cloud/storagefile.h
            cloud/downloadrequest.h
            cloud/folderdownloadrequest.h
            cloud/savessyncrequest.h
            cloud/box/boxstorage.h
            cloud/box/boxlistdirectorybyidrequest.h
            cloud/box/boxtokenrefresher.h
            cloud/box/boxuploadrequest.h
            cloud/dropbox/dropboxstorage.h
            cloud/dropbox/dropboxcreatedirectoryrequest.h
            cloud/dropbox/dropboxinforequest.h
            cloud/dropbox/dropboxlistdirectoryrequest.h
            cloud/dropbox/dropboxtokenrefresher.h
            cloud/dropbox/dropboxuploadrequest.h
            cloud/googledrive/googledrivelistdirectorybyidrequest.h
            cloud/googledrive/googledrivestorage.h
            cloud/googledrive/googledrivetokenrefresher.h
            cloud/googledrive/googledriveuploadrequest.h
            cloud/id/idstorage.h
            cloud/id/idcreatedirectoryrequest.h
            cloud/id/iddownloadrequest.h
            cloud/id/idlistdirectoryrequest.h
            cloud/id/idresolveidrequest.h
            cloud/id/idstreamfilerequest.h
            cloud/onedrive/onedrivestorage.h
            cloud/onedrive/onedrivecreatedirectoryrequest.h
            cloud/onedrive/onedrivetokenrefresher.h
            cloud/onedrive/onedrivelistdirectoryrequest.h
            cloud/onedrive/onedriveuploadrequest.h)
endif()

if(USE_SCUMMVMDLC)
  target_sources(
    backends
    PRIVATE dlc/scummvmcloud.cpp
    PRIVATE FILE_SET HEADERS FILES dlc/scummvmcloud.h)
endif()

if(USE_DLC)
  target_sources(
    backends
    PRIVATE dlc/dlcmanager.cpp
    PRIVATE FILE_SET HEADERS FILES dlc/dlcmanager.h)
endif()

if(USE_LIBCURL)
  target_sources(
    backends
    PRIVATE networking/basic/curl/cacert.cpp networking/basic/curl/socket.cpp
            networking/basic/curl/url.cpp
    PRIVATE FILE_SET HEADERS FILES networking/basic/curl/cacert.h
            networking/basic/curl/socket.h networking/basic/curl/url.h)
  if(USE_HTTP)
    target_sources(
      backends
      PRIVATE networking/http/curl/connectionmanager-curl.cpp
              networking/http/curl/networkreadstream-curl.cpp
      PRIVATE FILE_SET HEADERS FILES
              networking/http/curl/connectionmanager-curl.h
              networking/http/curl/networkreadstream-curl.h)
  endif()
endif()

if(EMSCRIPTEN)
  target_sources(
    backends
    PRIVATE fs/emscripten/emscripten-fs-factory.cpp
            fs/emscripten/emscripten-posix-fs.cpp fs/emscripten/http-fs.cpp
            midi/webmidi.cpp
    PRIVATE FILE_SET HEADERS FILES fs/emscripten/emscripten-fs-factory.h
            fs/emscripten/emscripten-posix-fs.h fs/emscripten/http-fs.h)
  if(USE_CLOUD)
    target_sources(
      backends
      PRIVATE fs/emscripten/cloud-fs.cpp
      PRIVATE FILE_SET HEADERS FILES fs/emscripten/cloud-fs.h)
  endif()

  if(USE_HTTP)
    target_sources(
      backends
      PRIVATE networking/http/emscripten/connectionmanager-emscripten.cpp
              networking/http/emscripten/networkreadstream-emscripten.cpp
      PRIVATE FILE_SET HEADERS FILES
              networking/http/emscripten/connectionmanager-emscripten.h
              networking/http/emscripten/networkreadstream-emscripten.h)
  endif()

  if(USE_TTS)
    target_sources(
      backends
      PRIVATE text-to-speech/emscripten/emscripten-text-to-speech.cpp
      PRIVATE FILE_SET HEADERS FILES
              text-to-speech/emscripten/emscripten-text-to-speech.h)
  endif()
endif()

if(USE_SDL_NET)
  target_sources(
    backends
    PRIVATE networking/sdl_net/client.cpp
            networking/sdl_net/getclienthandler.cpp
            networking/sdl_net/handlers/createdirectoryhandler.cpp
            networking/sdl_net/handlers/downloadfilehandler.cpp
            networking/sdl_net/handlers/filesajaxpagehandler.cpp
            networking/sdl_net/handlers/filesbasehandler.cpp
            networking/sdl_net/handlers/filespagehandler.cpp
            networking/sdl_net/handlers/indexpagehandler.cpp
            networking/sdl_net/handlers/listajaxhandler.cpp
            networking/sdl_net/handlers/resourcehandler.cpp
            networking/sdl_net/handlers/uploadfilehandler.cpp
            networking/sdl_net/handlerutils.cpp
            networking/sdl_net/localwebserver.cpp
            networking/sdl_net/reader.cpp
            networking/sdl_net/uploadfileclienthandler.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            networking/sdl_net/client.h
            networking/sdl_net/getclienthandler.h
            networking/sdl_net/handlers/createdirectoryhandler.h
            networking/sdl_net/handlers/downloadfilehandler.h
            networking/sdl_net/handlers/filesajaxpagehandler.h
            networking/sdl_net/handlers/filesbasehandler.h
            networking/sdl_net/handlers/filespagehandler.h
            networking/sdl_net/handlers/indexpagehandler.h
            networking/sdl_net/handlers/listajaxhandler.h
            networking/sdl_net/handlers/resourcehandler.h
            networking/sdl_net/handlers/uploadfilehandler.h
            networking/sdl_net/handlerutils.h
            networking/sdl_net/localwebserver.h
            networking/sdl_net/reader.h
            networking/sdl_net/uploadfileclienthandler.h)
  if(USE_CLOUD)
    target_sources(
      backends
      PRIVATE networking/sdl_net/handlers/connectcloudhandler.cpp
      PRIVATE FILE_SET HEADERS FILES
              networking/sdl_net/handlers/connectcloudhandler.h)
  endif()
endif()

if(USE_ENET)
  target_sources(
    backends
    PRIVATE networking/enet/source/callbacks.cpp
            networking/enet/source/compress.cpp
            networking/enet/source/host.cpp
            networking/enet/source/list.cpp
            networking/enet/source/packet.cpp
            networking/enet/source/peer.cpp
            networking/enet/source/protocol.cpp
    PRIVATE FILE_SET HEADERS FILES networking/enet/source/callbacks.h
            networking/enet/source/list.h networking/enet/source/protocol.h)
  if(WIN32)
    target_sources(
      backends
      PRIVATE networking/enet/source/win32.cpp
      PRIVATE FILE_SET HEADERS FILES networking/enet/source/win32.h)
  else()
    target_sources(
      backends
      PRIVATE networking/enet/source/unix.cpp
      PRIVATE FILE_SET HEADERS FILES networking/enet/source/unix.h)
  endif()

  # TODO: Is the order important here? If not, merge with the top block.
  target_sources(
    backends
    PRIVATE networking/enet/enet.cpp networking/enet/host.cpp
            networking/enet/socket.cpp
    PRIVATE FILE_SET HEADERS FILES networking/enet/enet.h
            networking/enet/host.h networking/enet/socket.h)
endif()

if(USE_ELF_LOADER)
  target_sources(
    backends
    PRIVATE plugins/elf/arm-loader.cpp
            plugins/elf/elf-loader.cpp
            plugins/elf/elf-provider.cpp
            plugins/elf/memory-manager.cpp
            plugins/elf/mips-loader.cpp
            plugins/elf/ppc-loader.cpp
            plugins/elf/shorts-segment-manager.cpp
            plugins/elf/version.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            plugins/elf/arm-loader.h
            plugins/elf/elf-loader.h
            plugins/elf/elf-provider.h
            plugins/elf/memory-manager.h
            plugins/elf/mips-loader.h
            plugins/elf/ppc-loader.h
            plugins/elf/shorts-segment-manager.h
            plugins/elf/version.h)
endif()

if(ENABLE_VKEYBD)
  target_sources(
    backends
    PRIVATE vkeybd/image-map.cpp vkeybd/polygon.cpp vkeybd/virtual-keyboard.cpp
            vkeybd/virtual-keyboard-gui.cpp vkeybd/virtual-keyboard-parser.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            vkeybd/image-map.h
            vkeybd/polygon.h
            vkeybd/virtual-keyboard.h
            vkeybd/virtual-keyboard-gui.h
            vkeybd/virtual-keyboard-parser.h)
endif()

# OpenGL specific source files.
if(USE_OPENGL)
  target_sources(
    backends
    PRIVATE graphics/opengl/framebuffer.cpp
            graphics/opengl/opengl-graphics.cpp
            graphics/opengl/renderer3d.cpp
            graphics/opengl/shader.cpp
            graphics/opengl/texture.cpp
            graphics/opengl/pipelines/clut8.cpp
            graphics/opengl/pipelines/fixed.cpp
            graphics/opengl/pipelines/pipeline.cpp
            graphics/opengl/pipelines/libretro.cpp
            graphics/opengl/pipelines/libretro/parser.cpp
            graphics/opengl/pipelines/shader.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            graphics/opengl/framebuffer.h
            graphics/opengl/opengl-graphics.h
            graphics/opengl/renderer3d.h
            graphics/opengl/shader.h
            graphics/opengl/texture.h
            graphics/opengl/pipelines/clut8.h
            graphics/opengl/pipelines/fixed.h
            graphics/opengl/pipelines/pipeline.h
            graphics/opengl/pipelines/libretro.h
            graphics/opengl/pipelines/libretro/parser.h
            graphics/opengl/pipelines/shader.h)
endif()

# SDL specific source files. We cannot just check $BACKEND = sdl, as various
# other backends derive from the SDL backend, and they all need the following
# files.
if(SDL_BACKEND)
  target_sources(
    backends
    PRIVATE events/sdl/sdl-common-events.cpp
            graphics/sdl/sdl-graphics.cpp
            graphics/surfacesdl/surfacesdl-graphics.cpp
            mixer/sdl/sdl-mixer.cpp
            mixer/null/null-mixer.cpp
            mutex/sdl/sdl-mutex.cpp
            timer/sdl/sdl-timer.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            graphics/sdl/sdl-graphics.h
            graphics/surfacesdl/surfacesdl-graphics.h
            mixer/sdl/sdl-mixer.h
            mixer/null/null-mixer.h
            mutex/sdl/sdl-mutex.h
            timer/sdl/sdl-timer.h)

  # TODO - We can do better with SDL versions in CMake
  if(NOT USE_SDL3 AND NOT USE_SDL2) # SDL 1
    target_sources(backends PRIVATE events/sdl/sdl1-events.cpp)
  endif()

  if(USE_SDL2)
    target_sources(backends PRIVATE events/sdl/sdl2-events.cpp)
  endif()

  if(USE_SDL3)
    target_sources(backends PRIVATE events/sdl/sdl3-events.cpp)
  endif()

  if(NOT RISCO AND NOT KOLIBRIOS)
    target_sources(
      backends
      PRIVATE plugins/sdl/sdl-provider.cpp
      PRIVATE FILE_SET HEADERS FILES plugins/sdl/sdl-provider.h)
  endif()

  # SDL 2 removed audio CD support
  if(NOT USE_SDL2)
    target_sources(
      backends
      PRIVATE audiocd/sdl/sdl-audiocd.cpp
      PRIVATE FILE_SET HEADERS FILES audiocd/sdl/sdl-audiocd.h)
  endif()

  if(USE_OPENGL)
    target_sources(
      backends
      PRIVATE graphics/openglsdl/openglsdl-graphics.cpp
      PRIVATE FILE_SET HEADERS FILES graphics/openglsdl/openglsdl-graphics.h)
  endif()

  if(USE_DISCORD)
    target_sources(
      backends
      PRIVATE presence/discord/discord.cpp
      PRIVATE FILE_SET HEADERS FILES presence/discord/discord.h)
  endif()
endif()

if(KOLIBRIOS)
  target_sources(
    backends
    PRIVATE fs/kolibrios/kolibrios-fs.cpp fs/kolibrios/kolibrios-fs-factory.cpp
            fs/posix/posix-iostream.cpp plugins/kolibrios/kolibrios-provider.cpp
            saves/kolibrios/kolibrios-saves.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            fs/kolibrios/kolibrios-fs.h
            fs/kolibrios/kolibrios-fs-factory.h
            fs/posix/posix-iostream.h
            plugins/kolibrios/kolibrios-provider.h
            saves/kolibrios/kolibrios-saves.h)
endif()

if(POSIX)
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
            dialogs/gtk/gtk-dialogs.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            fs/posix/posix-fs.h
            fs/posix/posix-fs-factory.h
            fs/posix/posix-iostream.h
            fs/posix-drives/posix-drives-fs.h
            fs/posix-drives/posix-drives-fs-factory.h
            fs/chroot/chroot-fs-factory.h
            fs/chroot/chroot-fs.h
            plugins/posix/posix-provider.h
            saves/posix/posix-saves.h
            taskbar/unity/unity-taskbar.h
            dialogs/gtk/gtk-dialogs.h)
  if(USE_SPEECH_DISPATCHER AND NOT USE_TTS)

    target_sources(
      backends
      PRIVATE text-to-speech/linux/linux-text-to-speech.cpp
      PRIVATE FILE_SET HEADERS FILES
              text-to-speech/linux/linux-text-to-speech.h)
  endif()
endif()

if(MACOSX)
  target_sources(
    backends
    PRIVATE audiocd/macosx/macosx-audiocd.cpp dialogs/macosx/macosx-dialogs.cpp
            midi/coreaudio.cpp midi/coremidi.cpp
            updates/macosx/macosx-updates.cpp taskbar/macosx/macosx-taskbar.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            audiocd/macosx/macosx-audiocd.h
            dialogs/macosx/macosx-dialogs.h
            updates/macosx/macosx-updates.h
            taskbar/macosx/macosx-taskbar.h)
  if(USE_TTS)
    target_sources(
      backends
      PRIVATE text-to-speech/macosx/macosx-text-to-speech.cpp
      PRIVATE FILE_SET HEADERS FILES
              text-to-speech/macosx/macosx-text-to-speech.h)
  endif()

  if(SDL_BACKEND)
    target_sources(
      backends
      PRIVATE plugins/sdl/macosx/macosx-provider.cpp
      PRIVATE FILE_SET HEADERS FILES plugins/sdl/macosx/macosx-provider.h)
  endif()

endif()

if(WIN32)
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
            taskbar/win32/win32-taskbar.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            audiocd/win32/win32-audiocd.h
            dialogs/win32/win32-dialogs.h
            fs/windows/windows-fs.h
            fs/windows/windows-fs-factory.h
            plugins/win32/win32-provider.h
            saves/windows/windows-saves.h
            updates/win32/win32-updates.h
            taskbar/win32/win32-taskbar.h)
  if(USE_TTS)
    target_sources(
      backends
      PRIVATE text-to-speech/windows/windows-text-to-speech.cpp
      PRIVATE FILE_SET HEADERS FILES
              text-to-speech/windows/windows-text-to-speech.h)
  endif()
endif()

if(BACKEND STREQUAL "3ds")
  target_sources(
    backends
    PRIVATE mutex/3ds/3ds-mutex.cpp
    PRIVATE FILE_SET HEADERS FILES mutex/3ds/3ds-mutex.h)
endif()

if(BACKEND STREQUAL "android")
  target_sources(
    backends
    PRIVATE fs/android/android-fs-factory.cpp
            fs/android/android-posix-fs.cpp
            fs/android/android-saf-fs.cpp
            graphics/android/android-graphics.cpp
            mutex/pthread/pthread-mutex.cpp
            networking/basic/android/jni.cpp
            networking/basic/android/socket.cpp
            networking/basic/android/url.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            fs/android/android-fs-factory.h
            fs/android/android-posix-fs.h
            fs/android/android-saf-fs.h
            graphics/android/android-graphics.h
            mutex/pthread/pthread-mutex.h
            networking/basic/android/jni.h
            networking/basic/android/socket.h
            networking/basic/android/url.h)
  if(USE_HTTP)
    target_sources(
      backends
      PRIVATE networking/http/android/connectionmanager-android.cpp
              networking/http/android/networkreadstream-android.cpp
      PRIVATE FILE_SET HEADERS FILES
              networking/http/android/connectionmanager-android.h
              networking/http/android/networkreadstream-android.h)
  endif()

endif()

if(AMIGAOS)
  target_sources(
    backends
    PRIVATE dialogs/amigaos/amigaos-dialogs.cpp fs/amigaos/amigaos-fs.cpp
            fs/amigaos/amigaos-fs-factory.cpp midi/camd.cpp
    PRIVATE FILE_SET HEADERS FILES dialogs/amigaos/amigaos-dialogs.h
            fs/amigaos/amigaos-fs.h fs/amigaos/amigaos-fs-factory.h)
endif()

if(MORPHOS)
  target_sources(
    backends
    PRIVATE fs/morphos/morphos-fs.cpp fs/morphos/morphos-fs-factory.cpp
            dialogs/morphos/morphos-dialogs.cpp midi/camd.cpp
    PRIVATE FILE_SET HEADERS FILES fs/morphos/morphos-fs.h
            fs/morphos/morphos-fs-factory.h dialogs/morphos/morphos-dialogs.h)
endif()

if(RISCOS)
  target_sources(
    backends
    PRIVATE events/riscossdl/riscossdl-events.cpp
            graphics/riscossdl/riscossdl-graphics.cpp
            fs/riscos/riscos-fs.cpp
            fs/riscos/riscos-fs-factory.cpp
            midi/riscos.cpp
            plugins/riscos/riscos-provider.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            events/riscossdl/riscossdl-events.h
            graphics/riscossdl/riscossdl-graphics.h
            fs/riscos/riscos-fs.h
            fs/riscos/riscos-fs-factory.h
            plugins/riscos/riscos-provider.h)
  if(NOT SDL_BACKEND)
    target_sources(
      backends
      PRIVATE platform/sdl/riscos/riscos-utils.cpp
      PRIVATE FILE_SET HEADERS FILES platform/sdl/riscos/riscos-utils.h)
  endif()

endif()

if(PLAYSTATION3)
  target_sources(
    backends
    PRIVATE fs/posix/posix-fs.cpp fs/posix/posix-fs-factory.cpp
            fs/posix/posix-iostream.cpp fs/ps3/ps3-fs-factory.cpp
            events/ps3sdl/ps3sdl-events.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            fs/posix/posix-fs.h
            fs/posix/posix-fs-factory.h
            fs/posix/posix-iostream.h
            fs/ps3/ps3-fs-factory.h
            events/ps3sdl/ps3sdl-events.h)
endif()

if(USE_LINUXCD)
  target_sources(
    backends
    PRIVATE audiocd/linux/linux-audiocd.cpp
    PRIVATE FILE_SET HEADERS FILES audiocd/linux/linux-audiocd.h)
endif()

if((BACKEND STREQUAL "3ds"))
  target_sources(
    backends
    PRIVATE plugins/3ds/3ds-provider.cpp
    PRIVATE FILE_SET HEADERS FILES plugins/3ds/3ds-provider.h)
endif()

if((BACKEND STREQUAL "atari"))
  target_sources(
    backends
    PRIVATE events/atari/atari-events.cpp
            fs/atari/atari-fs.cpp
            fs/atari/atari-fs-factory.cpp
            graphics/atari/atari-c2p-asm.cpp
            graphics/atari/atari-cursor.cpp
            graphics/atari/atari-graphics.cpp
            graphics/atari/atari-graphics-asm.cpp
            graphics/atari/atari-pendingscreenchanges.cpp
            graphics/atari/atari-screen.cpp
            graphics/atari/atari-supervidel.cpp
            graphics/atari/atari-surface.cpp
            mixer/atari/atari-mixer.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            events/atari/atari-events.h
            fs/atari/atari-fs.h
            fs/atari/atari-fs-factory.h
            graphics/atari/atari-c2p-asm.h
            graphics/atari/atari-cursor.h
            graphics/atari/atari-graphics.h
            graphics/atari/atari-graphics-asm.h
            graphics/atari/atari-pendingscreenchanges.h
            graphics/atari/atari-screen.h
            graphics/atari/atari-supervidel.h
            graphics/atari/atari-surface.h
            mixer/atari/atari-mixer.h)
endif()

if((BACKEND STREQUAL "ds"))
  target_sources(
    backends
    PRIVATE events/ds/ds-events.cpp
            fs/posix/posix-fs.cpp
            fs/posix/posix-fs-factory.cpp
            fs/posix/posix-iostream.cpp
            fs/posix-drives/posix-drives-fs.cpp
            fs/posix-drives/posix-drives-fs-factory.cpp
            fs/devoptab/devoptab-fs-factory.cpp
            mixer/maxmod/maxmod-mixer.cpp
            plugins/ds/ds-provider.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            events/ds/ds-events.h
            fs/posix/posix-fs.h
            fs/posix/posix-fs-factory.h
            fs/posix/posix-iostream.h
            fs/posix-drives/posix-drives-fs.h
            fs/posix-drives/posix-drives-fs-factory.h
            fs/devoptab/devoptab-fs-factory.h
            mixer/maxmod/maxmod-mixer.h
            plugins/ds/ds-provider.h)
endif()

if(IPHONE)
  target_sources(
    backends
    PRIVATE mutex/pthread/pthread-mutex.cpp graphics/ios/ios-graphics.cpp
            graphics/ios/renderbuffer.cpp
    PRIVATE FILE_SET HEADERS FILES mutex/pthread/pthread-mutex.h
            graphics/ios/ios-graphics.h graphics/ios/renderbuffer.h)
endif()

if((BACKEND STREQUAL "maemo"))
  target_sources(
    backends
    PRIVATE events/maemosdl/maemosdl-events.cpp
            graphics/maemosdl/maemosdl-graphics.cpp
    PRIVATE FILE_SET HEADERS FILES events/maemosdl/maemosdl-events.h
            graphics/maemosdl/maemosdl-graphics.h)
endif()

if((BACKEND STREQUAL "n64"))
  target_sources(
    backends
    PRIVATE fs/n64/n64-fs.cpp fs/n64/n64-fs-factory.cpp fs/n64/romfsstream.cpp
    PRIVATE FILE_SET HEADERS FILES fs/n64/n64-fs.h fs/n64/n64-fs-factory.h
            fs/n64/romfsstream.h)
endif()

if((BACKEND STREQUAL "null"))
  target_sources(
    backends
    PRIVATE mixer/null/null-mixer.cpp
    PRIVATE FILE_SET HEADERS FILES mixer/null/null-mixer.h)
endif()

if(MIYOO)
  if(($(MIYOO_TARGET) STREQUAL "miyoomini"))
    target_sources(
      backends
      PRIVATE graphics/miyoo/miyoomini-graphics.cpp
      PRIVATE FILE_SET HEADERS FILES graphics/miyoo/miyoomini-graphics.h)
  endif()

endif()

if(OPENDINGUX)
  target_sources(
    backends
    PRIVATE graphics/opendingux/opendingux-graphics.cpp
    PRIVATE FILE_SET HEADERS FILES graphics/opendingux/opendingux-graphics.h)
endif()

if((BACKEND STREQUAL "openpandora"))
  target_sources(
    backends
    PRIVATE events/openpandora/op-events.cpp
            graphics/openpandora/op-graphics.cpp
    PRIVATE FILE_SET HEADERS FILES events/openpandora/op-events.h
            graphics/openpandora/op-graphics.h)
endif()

if((BACKEND STREQUAL "psp"))
  target_sources(
    backends
    PRIVATE fs/psp/psp-fs.cpp fs/psp/psp-fs-factory.cpp fs/psp/psp-stream.cpp
            plugins/psp/psp-provider.cpp timer/psp/timer.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            fs/psp/psp-fs.h
            fs/psp/psp-fs-factory.h
            fs/psp/psp-stream.h
            plugins/psp/psp-provider.h
            timer/psp/timer.h)
endif()

if((BACKEND STREQUAL "psp2"))
  target_sources(
    backends
    PRIVATE fs/posix/posix-fs.cpp
            fs/posix/posix-iostream.cpp
            fs/posix-drives/posix-drives-fs.cpp
            fs/posix-drives/posix-drives-fs-factory.cpp
            plugins/psp2/psp2-provider.cpp
            events/psp2sdl/psp2sdl-events.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            fs/posix/posix-fs.h
            fs/posix/posix-iostream.h
            fs/posix-drives/posix-drives-fs.h
            fs/posix-drives/posix-drives-fs-factory.h
            plugins/psp2/psp2-provider.h
            events/psp2sdl/psp2sdl-events.h)
endif()

if((BACKEND STREQUAL "samsungtv"))
  target_sources(
    backends
    PRIVATE events/samsungtvsdl/samsungtvsdl-events.cpp
    PRIVATE FILE_SET HEADERS FILES events/samsungtvsdl/samsungtvsdl-events.h)
endif()

if((BACKEND STREQUAL "wii"))
  target_sources(
    backends
    PRIVATE fs/wii/wii-fs.cpp fs/wii/wii-fs-factory.cpp mutex/wii/wii-mutex.cpp
            plugins/wii/wii-provider.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            fs/wii/wii-fs.h
            fs/wii/wii-fs-factory.h
            mutex/wii/wii-mutex.h
            plugins/wii/wii-provider.h)
endif()

if((BACKEND STREQUAL "switch"))
  target_sources(
    backends
    PRIVATE events/switchsdl/switchsdl-events.cpp
    PRIVATE FILE_SET HEADERS FILES events/switchsdl/switchsdl-events.h)
endif()

if(ENABLE_EVENTRECORDER)
  target_sources(
    backends
    PRIVATE saves/recorder/recorder-saves.cpp
    PRIVATE FILE_SET HEADERS FILES saves/recorder/recorder-saves.h)
  if(NOT SDL_BACKEND)
    if(NOT (BACKEND STREQUAL "null"))
      target_sources(
        backends
        PRIVATE mixer/null/null-mixer.cpp
        PRIVATE FILE_SET HEADERS FILES mixer/null/null-mixer.h)
    endif()
  endif()
endif()

if(USE_IMGUI)
  target_sources(
    backends
    PRIVATE imgui/imgui.cpp
            imgui/imgui_demo.cpp
            imgui/imgui_draw.cpp
            imgui/imgui_fonts.cpp
            imgui/imgui_tables.cpp
            imgui/imgui_widgets.cpp
            imgui/imgui_utils.cpp
            imgui/components/imgui_logger.cpp
            imgui/misc/freetype/imgui_freetype.cpp
    PRIVATE FILE_SET
            HEADERS
            FILES
            imgui/imgui.h
            imgui/imgui_fonts.h
            imgui/imgui_utils.h
            imgui/components/imgui_logger.h
            imgui/misc/freetype/imgui_freetype.h)

  if(USE_OPENGL)
    target_sources(
      backends
      PRIVATE imgui/backends/imgui_impl_opengl3.cpp
      PRIVATE FILE_SET HEADERS FILES imgui/backends/imgui_impl_opengl3.h)
  endif()

  if(USE_SDL2)
    target_sources(
      backends
      PRIVATE imgui/backends/imgui_impl_sdl2.cpp
      PRIVATE FILE_SET HEADERS FILES imgui/backends/imgui_impl_sdl2.h)

    if(USE_IMGUI_SDLRENDERER2)
      target_sources(
        backends
        PRIVATE imgui/backends/imgui_impl_sdlrenderer2.cpp
        PRIVATE FILE_SET HEADERS FILES imgui/backends/imgui_impl_sdlrenderer2.h)
    endif()
  endif()

  if(USE_SDL3)
    target_sources(
      backends
      PRIVATE imgui/backends/imgui_impl_sdl3.cpp
      PRIVATE FILE_SET HEADERS FILES imgui/backends/imgui_impl_sdl3.h)

    if(USE_IMGUI_SDLRENDERER3)
      target_sources(
        backends
        PRIVATE imgui/backends/imgui_impl_sdlrenderer3.cpp
        PRIVATE FILE_SET HEADERS FILES imgui/backends/imgui_impl_sdlrenderer3.h)
    endif()
  endif()
endif()
