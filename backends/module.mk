MODULE := backends

MODULE_OBJS := \
	base-backend.o \
	events/default/default-events.o \
	fs/abstract-fs.o \
	fs/stdiostream.o \
	fs/amigaos4/amigaos4-fs-factory.o \
	fs/posix/posix-fs-factory.o \
	fs/symbian/symbian-fs-factory.o \
	fs/windows/windows-fs-factory.o \
	keymapper/action.o \
	keymapper/keymap.o \
	keymapper/keymapper.o \
	keymapper/remap-dialog.o \
	midi/alsa.o \
	midi/camd.o \
	midi/coreaudio.o \
	midi/coremidi.o \
	midi/seq.o \
	midi/stmidi.o \
	midi/timidity.o \
	midi/dmedia.o \
	midi/windows.o \
	plugins/posix/posix-provider.o \
	plugins/sdl/sdl-provider.o \
	plugins/win32/win32-provider.o \
	saves/savefile.o \
	saves/default/default-saves.o \
	saves/posix/posix-saves.o \
	timer/default/default-timer.o \
	vkeybd/image-map.o \
	vkeybd/polygon.o \
	vkeybd/virtual-keyboard.o \
	vkeybd/virtual-keyboard-gui.o \
	vkeybd/virtual-keyboard-parser.o

ifeq ($(BACKEND),dc)
MODULE_OBJS += \
	plugins/dc/dc-provider.o
endif

ifeq ($(BACKEND),ds)
MODULE_OBJS += \
	fs/ds/ds-fs-factory.o \
	fs/ds/ds-fs.o
endif

ifeq ($(BACKEND),n64)
MODULE_OBJS += \
	fs/n64/n64-fs-factory.o \
	fs/n64/romfsstream.o
endif

ifeq ($(BACKEND),ps2)
MODULE_OBJS += \
	fs/ps2/ps2-fs-factory.o
endif

ifeq ($(BACKEND),psp)
MODULE_OBJS += \
	fs/psp/psp-fs-factory.o \
	fs/psp/psp-stream.o \
	plugins/psp/psp-provider.o \
	saves/psp/psp-saves.o \
	timer/psp/timer.o
endif

ifeq ($(BACKEND),wii)
MODULE_OBJS += \
	fs/wii/wii-fs-factory.o
endif

# Include common rules
include $(srcdir)/rules.mk
