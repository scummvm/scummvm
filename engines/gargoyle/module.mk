MODULE := engines/gargoyle

MODULE_OBJS := \
	detection.o \
	events.o \
	gargoyle.o \
	glk.o \
	interpreter.o \
	scott/scott.o

# This module can be built as a plugin
ifeq ($(ENABLE_GARGOYLE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
