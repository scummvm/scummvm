MODULE := backends

MODULE_OBJS = \
	backends/fs/posix/posix-fs.o \
	backends/fs/morphos/abox-fs.o \
	backends/fs/windows/windows-fs.o

# Include common rules 
include common.rules
