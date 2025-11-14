MODULE := engines/phoenixvr

MODULE_OBJS = \
	pakf.o \
	phoenixvr.o \
	console.o \
	metaengine.o \
	script.o

# This module can be built as a plugin
ifeq ($(ENABLE_PHOENIXVR), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
