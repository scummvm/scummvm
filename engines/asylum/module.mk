MODULE := engines/asylum

MODULE_OBJS := \
	detection.o \
	graphics.o \
	resource.o \
	screen.o \
	asylum.o

# This module can be built as a plugin
ifeq ($(ENABLE_ASYLUM), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
