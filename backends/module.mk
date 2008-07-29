MODULE := backends

MODULE_OBJS := \
	fs/amigaos4/amigaos4-fs-factory.o \
	fs/ds/ds-fs-factory.o \
	fs/palmos/palmos-fs-factory.o \
	fs/posix/posix-fs-factory.o \
	fs/ps2/ps2-fs-factory.o \
	fs/psp/psp-fs-factory.o \
	fs/symbian/symbian-fs-factory.o \
	fs/windows/windows-fs-factory.o \
	fs/wii/wii-fs-factory.o \
	events/default/default-events.o \
	midi/alsa.o \
	midi/camd.o \
	midi/coreaudio.o \
	midi/coremidi.o \
	midi/quicktime.o \
	midi/seq.o \
	midi/timidity.o \
	midi/dmedia.o \
	midi/windows.o \
	plugins/dc/dc-provider.o \
	plugins/posix/posix-provider.o \
	plugins/sdl/sdl-provider.o \
	plugins/win32/win32-provider.o \
	saves/savefile.o \
	saves/default/default-saves.o \
	saves/compressed/compressed-saves.o \
	timer/default/default-timer.o \
	common/virtual-keyboard.o \
	common/virtual-keyboard-parser.o \
	common/keymap.o \
	common/keymap-manager.o \
	common/keymapper.o \
	common/user-action.o \

# Include common rules
include $(srcdir)/rules.mk
