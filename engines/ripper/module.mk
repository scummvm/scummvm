MODULE := engines/ripper

MODULE_OBJS := \
	cursor.o \
	iff.o \
	input.o \
	media.o \
	menu.o \
	metaengine.o \
	resources.o \
	ripper.o \
	script.o

# This module can be built as a plugin
ifeq ($(ENABLE_RIPPER), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
