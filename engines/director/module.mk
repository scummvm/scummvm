MODULE := engines/director

MODULE_OBJS = \
	detection.o \
	director.o \
	resource.o \
	dib.o
# This module can be built as a plugin
ifeq ($(ENABLE_DIRECTOR), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
