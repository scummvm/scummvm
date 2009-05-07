MODULE := backends

MODULE_OBJS := \
	base-backend.o \
	events/default/default-events.o \
	fs/abstract-fs.o \
	fs/stdiostream.o \
	fs/amigaos4/amigaos4-fs-factory.o \
	fs/posix/posix-fs-factory.o \
	fs/windows/windows-fs-factory.o \
	saves/savefile.o \
	saves/default/default-saves.o \
	saves/posix/posix-saves.o \
	timer/default/default-timer.o

# Include common rules
include $(srcdir)/rules.mk
