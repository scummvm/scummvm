MODULE := engines/cryo

MODULE_OBJS = \
	cryo.o \
	eden.o \
	detection.o

# This module can be built as a plugin
ifeq ($(ENABLE_CRYO), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
