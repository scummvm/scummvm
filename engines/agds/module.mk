MODULE := engines/agds

MODULE_OBJS := \
	agds.o \
	database.o \
	detection.o \
	mjpgPlayer.o \
	object.o \
	process.o \
	process_opcodes.o \
	region.o \
	resourceManager.o

# This module can be built as a plugin
ifeq ($(ENABLE_AGDS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
