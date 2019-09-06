MODULE := backends

MODULE_OBJS := \
	base-backend.o \
	modular-backend.o \
	audiocd/audiocd-stream.o \
	audiocd/default/default-audiocd.o \
	events/default/default-events.o \
	fs/abstract-fs.o \
	fs/stdiostream.o \
	log/log.o \
	midi/alsa.o \
	midi/dmedia.o \
	midi/seq.o \
	midi/sndio.o \
	midi/stmidi.o \
	midi/timidity.o \
	saves/savefile.o \
	saves/default/default-saves.o \
	timer/default/default-timer.o

ifdef USE_CLOUD

ifdef USE_LIBCURL
MODULE_OBJS += \
	cloud/basestorage.o \
	cloud/cloudicon.o \
	cloud/cloudmanager.o \
	cloud/iso8601.o \
	cloud/storage.o \
	cloud/storagefile.o \
	cloud/downloadrequest.o \
	cloud/folderdownloadrequest.o \
	cloud/savessyncrequest.o \
	cloud/box/boxstorage.o \
	cloud/box/boxlistdirectorybyidrequest.o \
	cloud/box/boxtokenrefresher.o \
	cloud/box/boxuploadrequest.o \
	cloud/dropbox/dropboxstorage.o \
	cloud/dropbox/dropboxcreatedirectoryrequest.o \
	cloud/dropbox/dropboxinforequest.o \
	cloud/dropbox/dropboxlistdirectoryrequest.o \
	cloud/dropbox/dropboxuploadrequest.o \
	cloud/googledrive/googledrivelistdirectorybyidrequest.o \
	cloud/googledrive/googledrivestorage.o \
	cloud/googledrive/googledrivetokenrefresher.o \
	cloud/googledrive/googledriveuploadrequest.o \
	cloud/id/idstorage.o \
	cloud/id/idcreatedirectoryrequest.o \
	cloud/id/iddownloadrequest.o \
	cloud/id/idlistdirectoryrequest.o \
	cloud/id/idresolveidrequest.o \
	cloud/id/idstreamfilerequest.o \
	cloud/onedrive/onedrivestorage.o \
	cloud/onedrive/onedrivecreatedirectoryrequest.o \
	cloud/onedrive/onedrivetokenrefresher.o \
	cloud/onedrive/onedrivelistdirectoryrequest.o \
	cloud/onedrive/onedriveuploadrequest.o
endif
endif

ifdef USE_LIBCURL
MODULE_OBJS += \
	networking/curl/connectionmanager.o \
	networking/curl/networkreadstream.o \
	networking/curl/curlrequest.o \
	networking/curl/curljsonrequest.o \
	networking/curl/request.o
endif

ifdef USE_SDL_NET
MODULE_OBJS += \
	networking/sdl_net/client.o \
	networking/sdl_net/getclienthandler.o \
	networking/sdl_net/handlers/createdirectoryhandler.o \
	networking/sdl_net/handlers/downloadfilehandler.o \
	networking/sdl_net/handlers/filesajaxpagehandler.o \
	networking/sdl_net/handlers/filesbasehandler.o \
	networking/sdl_net/handlers/filespagehandler.o \
	networking/sdl_net/handlers/indexpagehandler.o \
	networking/sdl_net/handlers/listajaxhandler.o \
	networking/sdl_net/handlers/resourcehandler.o \
	networking/sdl_net/handlers/uploadfilehandler.o \
	networking/sdl_net/handlerutils.o \
	networking/sdl_net/localwebserver.o \
	networking/sdl_net/reader.o \
	networking/sdl_net/uploadfileclienthandler.o
endif

ifdef USE_ELF_LOADER
MODULE_OBJS += \
	plugins/elf/arm-loader.o \
	plugins/elf/elf-loader.o \
	plugins/elf/elf-provider.o \
	plugins/elf/memory-manager.o \
	plugins/elf/mips-loader.o \
	plugins/elf/ppc-loader.o \
	plugins/elf/shorts-segment-manager.o \
	plugins/elf/version.o
endif

ifdef ENABLE_KEYMAPPER
MODULE_OBJS += \
	keymapper/action.o \
	keymapper/hardware-input.o \
	keymapper/keymap.o \
	keymapper/keymapper.o \
	keymapper/remap-dialog.o
endif

ifdef ENABLE_VKEYBD
MODULE_OBJS += \
	vkeybd/image-map.o \
	vkeybd/polygon.o \
	vkeybd/virtual-keyboard.o \
	vkeybd/virtual-keyboard-gui.o \
	vkeybd/virtual-keyboard-parser.o
endif

# OpenGL specific source files.
ifdef USE_OPENGL
MODULE_OBJS += \
	graphics/opengl/context.o \
	graphics/opengl/debug.o \
	graphics/opengl/framebuffer.o \
	graphics/opengl/opengl-graphics.o \
	graphics/opengl/shader.o \
	graphics/opengl/texture.o \
	graphics/opengl/pipelines/clut8.o \
	graphics/opengl/pipelines/fixed.o \
	graphics/opengl/pipelines/pipeline.o \
	graphics/opengl/pipelines/shader.o
endif

# SDL specific source files.
# We cannot just check $BACKEND = sdl, as various other backends
# derive from the SDL backend, and they all need the following files.
ifdef SDL_BACKEND
MODULE_OBJS += \
	events/sdl/sdl-events.o \
	graphics/sdl/sdl-graphics.o \
	graphics/surfacesdl/surfacesdl-graphics.o \
	mixer/sdl/sdl-mixer.o \
	mutex/sdl/sdl-mutex.o \
	plugins/sdl/sdl-provider.o \
	timer/sdl/sdl-timer.o

# SDL 2 removed audio CD support
ifndef USE_SDL2
MODULE_OBJS += \
	audiocd/sdl/sdl-audiocd.o
endif

ifdef USE_OPENGL
MODULE_OBJS += \
	graphics/openglsdl/openglsdl-graphics.o
endif
endif

ifdef POSIX
MODULE_OBJS += \
	fs/posix/posix-fs.o \
	fs/posix/posix-fs-factory.o \
	fs/chroot/chroot-fs-factory.o \
	fs/chroot/chroot-fs.o \
	plugins/posix/posix-provider.o \
	saves/posix/posix-saves.o \
	taskbar/unity/unity-taskbar.o

ifdef USE_SPEECH_DISPATCHER
ifdef USE_TTS
MODULE_OBJS += \
	text-to-speech/linux/linux-text-to-speech.o
endif
endif

endif

ifdef MACOSX
MODULE_OBJS += \
	audiocd/macosx/macosx-audiocd.o \
	dialogs/macosx/macosx-dialogs.o \
	midi/coreaudio.o \
	midi/coremidi.o \
	updates/macosx/macosx-updates.o \
	taskbar/macosx/macosx-taskbar.o

ifdef USE_TTS
MODULE_OBJS += \
	text-to-speech/macosx/macosx-text-to-speech.o
endif

endif

ifdef WIN32
MODULE_OBJS += \
	audiocd/win32/win32-audiocd.o \
	dialogs/win32/win32-dialogs.o \
	fs/windows/windows-fs.o \
	fs/windows/windows-fs-factory.o \
	midi/windows.o \
	plugins/win32/win32-provider.o \
	saves/windows/windows-saves.o \
	updates/win32/win32-updates.o \
	taskbar/win32/win32-taskbar.o

ifdef USE_TTS
MODULE_OBJS += \
	text-to-speech/windows/windows-text-to-speech.o
endif

endif

ifeq ($(BACKEND),android)
MODULE_OBJS += \
	mutex/pthread/pthread-mutex.o
endif

ifeq ($(BACKEND),androidsdl)
MODULE_OBJS += \
	events/androidsdl/androidsdl-events.o \
	graphics/androidsdl/androidsdl-graphics.o
endif

ifdef AMIGAOS
MODULE_OBJS += \
	fs/amigaos4/amigaos4-fs.o \
	fs/amigaos4/amigaos4-fs-factory.o \
	midi/camd.o
endif

ifdef RISCOS
MODULE_OBJS += \
	events/riscossdl/riscossdl-events.o \
	fs/riscos/riscos-fs.o \
	fs/riscos/riscos-fs-factory.o \
	platform/sdl/riscos/riscos-utils.o
endif

ifdef PLAYSTATION3
MODULE_OBJS += \
	fs/posix/posix-fs.o \
	fs/posix/posix-fs-factory.o \
	fs/ps3/ps3-fs-factory.o \
	events/ps3sdl/ps3sdl-events.o
endif

ifdef USE_LINUXCD
MODULE_OBJS += \
	audiocd/linux/linux-audiocd.o
endif

ifeq ($(BACKEND),tizen)
MODULE_OBJS += \
	timer/tizen/timer.o
endif

ifeq ($(BACKEND),ds)
MODULE_OBJS += \
	fs/ds/ds-fs.o \
	fs/ds/ds-fs-factory.o \
	plugins/ds/ds-provider.o
endif

ifeq ($(BACKEND),dingux)
MODULE_OBJS += \
	events/dinguxsdl/dinguxsdl-events.o \
	graphics/dinguxsdl/dinguxsdl-graphics.o
endif

ifeq ($(BACKEND),gph)
MODULE_OBJS += \
	events/gph/gph-events.o \
	graphics/gph/gph-graphics.o
endif

ifeq ($(BACKEND),linuxmoto)
MODULE_OBJS += \
	events/linuxmotosdl/linuxmotosdl-events.o \
	graphics/linuxmotosdl/linuxmotosdl-graphics.o
endif

ifeq ($(BACKEND),maemo)
MODULE_OBJS += \
	events/maemosdl/maemosdl-events.o \
	graphics/maemosdl/maemosdl-graphics.o
endif

ifeq ($(BACKEND),n64)
MODULE_OBJS += \
	fs/n64/n64-fs.o \
	fs/n64/n64-fs-factory.o \
	fs/n64/romfsstream.o
endif

ifeq ($(BACKEND),openpandora)
MODULE_OBJS += \
	events/openpandora/op-events.o \
	graphics/openpandora/op-graphics.o
endif

ifeq ($(BACKEND),ps2)
MODULE_OBJS += \
	fs/ps2/ps2-fs.o \
	fs/ps2/ps2-fs-factory.o \
	plugins/ps2/ps2-provider.o
endif

ifeq ($(BACKEND),psp)
MODULE_OBJS += \
	fs/psp/psp-fs.o \
	fs/psp/psp-fs-factory.o \
	fs/psp/psp-stream.o \
	plugins/psp/psp-provider.o \
	timer/psp/timer.o
endif

ifeq ($(BACKEND),psp2)
MODULE_OBJS += \
	fs/posix/posix-fs.o \
	fs/psp2/psp2-fs-factory.o \
	fs/psp2/psp2-dirent.o \
	events/psp2sdl/psp2sdl-events.o \
	graphics/psp2sdl/psp2sdl-graphics.o
endif

ifeq ($(BACKEND),samsungtv)
MODULE_OBJS += \
	events/samsungtvsdl/samsungtvsdl-events.o \
	graphics/samsungtvsdl/samsungtvsdl-graphics.o
endif

ifeq ($(BACKEND),webos)
MODULE_OBJS += \
	events/webossdl/webossdl-events.o
endif

ifeq ($(BACKEND),wince)
MODULE_OBJS += \
	events/wincesdl/wincesdl-events.o \
	fs/windows/windows-fs.o \
	fs/windows/windows-fs-factory.o \
	graphics/wincesdl/wincesdl-graphics.o \
	mixer/wincesdl/wincesdl-mixer.o \
	plugins/win32/win32-provider.o
endif

ifeq ($(BACKEND),wii)
MODULE_OBJS += \
	fs/wii/wii-fs.o \
	fs/wii/wii-fs-factory.o \
	plugins/wii/wii-provider.o
endif

ifeq ($(BACKEND),switch)
MODULE_OBJS += \
	events/switchsdl/switchsdl-events.o
endif

ifdef ENABLE_EVENTRECORDER
MODULE_OBJS += \
	mixer/nullmixer/nullsdl-mixer.o \
	saves/recorder/recorder-saves.o
endif

# Include common rules
include $(srcdir)/rules.mk
