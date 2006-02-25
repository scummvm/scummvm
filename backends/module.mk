MODULE := backends

MODULE_OBJS := \
	fs/fs.o \
	fs/posix/posix-fs.o \
	fs/morphos/abox-fs.o \
	fs/windows/windows-fs.o \
	fs/amigaos4/amigaos4-fs.o \
	midi/alsa.o \
	midi/coreaudio.o \
	midi/coremidi.o \
	midi/morphos.o \
	midi/quicktime.o \
	midi/seq.o \
	midi/windows.o

MODULE_DIRS += \
	backends \
	backends/fs \
	backends/fs/posix \
	backends/fs/morphos \
	backends/fs/windows \
	backends/fs/amigaos4 \
	backends/midi

# Include common rules 
include $(srcdir)/common.rules
