MODULE := backends

MODULE_OBJS = \
	backends/fs/posix/posix-fs.o \
	backends/fs/morphos/abox-fs.o \
	backends/fs/windows/windows-fs.o \
	backends/midi/adlib.o \
	backends/midi/coreaudio.o \
	backends/midi/morphos.o \
	backends/midi/null.o \
	backends/midi/quicktime.o \
	backends/midi/alsa.o \
	backends/midi/windows.o

# Include common rules 
include common.rules
