MODULE := engines/mutationofjb

MODULE_OBJS := \
	commands/command.o \
	commands/conditionalcommand.o \
	commands/endblockcommand.o \
	commands/ifcommand.o \
	commands/seqcommand.o \
	detection.o \
	encryptedfile.o \
	game.o \
	mutationofjb.o \
	room.o \
	script.o \
	util.o

# This module can be built as a plugin
ifeq ($(ENABLE_MUTATIONOFJB), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
