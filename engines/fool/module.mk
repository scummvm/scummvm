MODULE := engines/fool

MODULE_OBJS = \
	fool.o \
	fool_prologue.o \
	toolbox.o \
	zbasic.o \
	console.o \
	messages.o \
	metaengine.o

# This module can be built as a plugin
ifeq ($(ENABLE_FOOL), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
