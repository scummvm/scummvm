MODULE := engines/buried

MODULE_OBJS = \
	buried.o \
	detection.o \
	window.o


# This module can be built as a plugin
ifeq ($(ENABLE_BURIED), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
