MODULE := engines/hopkins

MODULE_OBJS := \
	detection.o \
	events.o \
	files.o \
	graphics.o \
	globals.o \
	hopkins.o \
	sound.o

# This module can be built as a plugin
ifeq ($(ENABLE_HOPKINS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
