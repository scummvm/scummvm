MODULE := engines/xeen

MODULE_OBJS := \
	debugger.o \
	detection.o \
	resources.o \
	xeen.o

# This module can be built as a plugin
ifeq ($(ENABLE_XEEN), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
