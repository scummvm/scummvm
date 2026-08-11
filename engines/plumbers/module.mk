MODULE := engines/plumbers

MODULE_OBJS = \
	3do.o \
	console.o \
	metaengine.o \
	plumbers.o \
	windows.o

# This module can be built as a plugin
ifeq ($(ENABLE_PLUMBERS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
