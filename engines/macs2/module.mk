MODULE := engines/macs2

MODULE_OBJS = \
	macs2.o \
	console.o \
	events.o \
	messages.o \
	metaengine.o \
	view1.o

# This module can be built as a plugin
ifeq ($(ENABLE_MACS2), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
