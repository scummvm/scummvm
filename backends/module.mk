MODULE := backends

MODULE_OBJS := \
	modular-backend.o \
	audiocd/default/default-audiocd.o \
	events/default/default-events.o \
	fs/abstract-fs.o \
	fs/stdiostream.o \
	keymapper/action.o \
	keymapper/keymap.o \
	keymapper/keymapper.o \
	keymapper/remap-dialog.o \
	log/log.o \
	midi/alsa.o \
	midi/dmedia.o \
	midi/seq.o \
	midi/stmidi.o \
	midi/timidity.o \
	saves/savefile.o \
	saves/default/default-saves.o \
	timer/default/default-timer.o \
	vkeybd/image-map.o \
	vkeybd/polygon.o \
	vkeybd/virtual-keyboard.o \
	vkeybd/virtual-keyboard-gui.o \
	vkeybd/virtual-keyboard-parser.o

# SDL specific source files.
# We cannot just check $BACKEND = sdl, as various other backends
# derive from the SDL backend, and they all need the following files.
# TODO: Add SDL_BACKEND to config.mk; this would match the fact that
# we also add -DSDL_BACKEND to the DEFINES.
# However, the latter is only done for *most* SDL based stuff, not always
# so we really should unify the relevant code in configure.
MODULE_OBJS += \
	audiocd/sdl/sdl-audiocd.o \
	events/sdl/sdl-events.o \
	graphics/sdl/sdl-graphics.o \
	mixer/doublebuffersdl/doublebuffersdl-mixer.o \
	mixer/sdl/sdl-mixer.o \
	mutex/sdl/sdl-mutex.o \
	plugins/sdl/sdl-provider.o \
	timer/sdl/sdl-timer.o

ifdef UNIX
MODULE_OBJS += \
	fs/posix/posix-fs.o \
	fs/posix/posix-fs-factory.o \
	plugins/posix/posix-provider.o \
	saves/posix/posix-saves.o
endif

ifdef MACOSX
MODULE_OBJS += \
	midi/coreaudio.o \
	midi/coremidi.o
endif

ifdef WIN32
MODULE_OBJS += \
	fs/windows/windows-fs.o \
	fs/windows/windows-fs-factory.o \
	midi/windows.o \
	plugins/win32/win32-provider.o
endif

ifdef AMIGAOS
MODULE_OBJS += \
	fs/amigaos4/amigaos4-fs.o \
	fs/amigaos4/amigaos4-fs-factory.o \
	midi/camd.o
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

# TODO/FIXME: The gp2xsdl files are only compiled if GP2X_OLD is defined,
# which currently is never the case (unless the user manually requests it).
# ifeq ($(BACKEND),gp2x)
# MODULE_OBJS += \
# 	events/gp2xsdl/gp2xsdl-events.o \
# 	graphics/gp2xsdl/gp2xsdl-graphics.o
# endif

ifeq ($(BACKEND),linuxmoto)
MODULE_OBJS += \
	events/linuxmotosdl/linuxmotosdl-events.o \
	graphics/linuxmotosdl/linuxmotosdl-graphics.o
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
	saves/psp/psp-saves.o \
	timer/psp/timer.o
endif

ifeq ($(BACKEND),samsungstv)
MODULE_OBJS += \
	events/samsungtvsdl/samsungtvsdl-events.o
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

# Include common rules
include $(srcdir)/rules.mk
