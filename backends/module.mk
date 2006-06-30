MODULE := backends

MODULE_OBJS := \
	fs/posix/posix-fs.o \
	fs/morphos/abox-fs.o \
	fs/windows/windows-fs.o \
	fs/amigaos4/amigaos4-fs.o \
	fs/dc/dc-fs.o \
	midi/alsa.o \
	midi/coreaudio.o \
	midi/coremidi.o \
	midi/morphos.o \
	midi/quicktime.o \
	midi/seq.o \
	midi/windows.o \
	saves/savefile.o \
	saves/default/default-saves.o \
	timer/default/default-timer.o

# Include common rules 
include $(srcdir)/rules.mk
