MODULE := engines/queen

MODULE_OBJS := \
	bankman.o \
	command.o \
	credits.o \
	cutaway.o \
	debug.o \
	display.o \
	graphics.o \
	grid.o \
	input.o \
	journal.o \
	logic.o \
	metaengine.o \
	midiadlib.o \
	music.o \
	musicdata.o \
	queen.o \
	resource.o \
	restables.o \
	sound.o \
	state.o \
	talk.o \
	walk.o

# This module can be built as a plugin
ifeq ($(ENABLE_QUEEN), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objecs
DETECT_OBJS += $(MODULE)/detection.o

# Skip building the following objects if a static
# module is enabled, because it already has the contents.
ifneq ($(ENABLE_QUEEN), STATIC_PLUGIN)
# External dependencies for detection.
DETECT_OBJS += $(MODULE)/resource.o
DETECT_OBJS += $(MODULE)/restables.o
endif
