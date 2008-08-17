MODULE := engine/backend

MODULE_OBJS := \
	events/default/default-events.o \
	fs/amigaos4/amigaos4-fs-factory.o \
	fs/posix/posix-fs-factory.o \
	fs/windows/windows-fs-factory.o \
	saves/savefile.o \
	saves/default/default-saves.o \
	saves/compressed/compressed-saves.o \
	timer/default/default-timer.o

# Include common rules
include $(srcdir)/rules.mk
