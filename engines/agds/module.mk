MODULE := engines/agds

MODULE_OBJS := \
	agds.o \
	animation.o \
	character.o \
	database.o \
	detection.o \
	font.o \
	inventory.o \
	mjpgPlayer.o \
	object.o \
	process.o \
	process_opcodes.o \
	region.o \
	resourceManager.o \
	screen.o \
	soundManager.o \
	systemVariable.o

# This module can be built as a plugin
ifeq ($(ENABLE_AGDS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
