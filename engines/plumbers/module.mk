MODULE := engines/plumbers

MODULE_OBJS = \
	plumbers.o \
	console.o \
	metaengine.o \
	windows.o

ifdef USE_RGB_COLOR
MODULE_OBJS += 3do.o
endif

# This module can be built as a plugin
ifeq ($(ENABLE_PLUMBERS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
