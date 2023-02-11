MODULE := engines/vcruise

MODULE_OBJS = \
	metaengine.o \
	runtime.o \
	script.o \
	textparser.o \
	vcruise.o


# This module can be built as a plugin
ifeq ($(ENABLE_VCRUISE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
