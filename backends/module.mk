MODULE := backends

MODULE_OBJS := \
	backends/fs/posix/posix-fs.o \
	backends/fs/morphos/abox-fs.o \
	backends/fs/windows/windows-fs.o \
	backends/midi/adlib.o \
	backends/midi/coreaudio.o \
	backends/midi/morphos.o \
	backends/midi/null.o \
	backends/midi/quicktime.o \
	backends/midi/seq.o \
	backends/midi/alsa.o \
	backends/midi/windows.o \
	backends/midi/ym2612.o

MODULE_DIRS += \
	backends \
	backends/fs/posix \
	backends/fs/morphos \
	backends/fs/windows \
	backends/midi

# Include common rules 
include $(srcdir)/common.rules
