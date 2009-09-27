MODULE := backends

MODULE_OBJS := \
	base-backend.o \
	events/default/default-events.o \
	fs/abstract-fs.o \
	fs/stdiostream.o \
	fs/amigaos4/amigaos4-fs-factory.o \
	fs/ds/ds-fs-factory.o \
	fs/palmos/palmos-fs-factory.o \
	fs/posix/posix-fs-factory.o \
	fs/ps2/ps2-fs-factory.o \
	fs/psp/psp-fs-factory.o \
	fs/psp/psp-stream.o \
	fs/symbian/symbian-fs-factory.o \
	fs/windows/windows-fs-factory.o \
	fs/wii/wii-fs-factory.o \
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
	plugins/dc/dc-provider.o \
	plugins/posix/posix-provider.o \
	plugins/sdl/sdl-provider.o \
	plugins/win32/win32-provider.o \
	saves/savefile.o \
	saves/default/default-saves.o \
	saves/posix/posix-saves.o \
	saves/psp/psp-saves.o \
	timer/default/default-timer.o \
	vkeybd/image-map.o \
	vkeybd/polygon.o \
	vkeybd/virtual-keyboard.o \
	vkeybd/virtual-keyboard-gui.o \
	vkeybd/virtual-keyboard-parser.o

# Include common rules
include $(srcdir)/rules.mk
