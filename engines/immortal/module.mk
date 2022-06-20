MODULE := engines/immortal

MODULE_OBJS = \
	immortal.o \
	disk.o \
	metaengine.o \
	compression.o

# This module can be built as a plugin
ifeq ($(ENABLE_IMMORTAL), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
