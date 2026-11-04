MODULE := engines/olga

MODULE_OBJS = \
	console.o \
	lbm.o \
	metaengine.o \
	olga.o

# This module can be built as a plugin
ifeq ($(ENABLE_OLGA), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
