MODULE := engines/lilliput

MODULE_OBJS = \
	console.o \
	lilliput.o \
	metaengine.o \
	script.o \
	sound.o \
	stream.o

MODULE_DIRS += \
	engines/lilliput

# This module can be built as a plugin
ifeq ($(ENABLE_LILLIPUT), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
