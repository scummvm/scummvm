MODULE := backends

MODULE_OBJS := \
	events/default/default-events.o \
	fs/abstract-fs-factory.o \
	midi/alsa.o \
	midi/coreaudio.o \
	midi/coremidi.o \
	midi/morphos.o \
	midi/quicktime.o \
	midi/seq.o \
	midi/windows.o \
	plugins/dc/dc-provider.o \
	plugins/posix/posix-provider.o \
	plugins/sdl/sdl-provider.o \
	plugins/win32/win32-provider.o \
	saves/savefile.o \
	saves/default/default-saves.o \
	saves/compressed/compressed-saves.o \
	timer/default/default-timer.o

# Include common rules 
include $(srcdir)/rules.mk
