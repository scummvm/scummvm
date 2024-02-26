MODULE := engines/bagel

MODULE_OBJS = \
	bagel.o \
	bof_error.o \
	console.o \
	metaengine.o \
	boflib/bof_list.o \
	boflib/llist.o \
	boflib/misc.o

# This module can be built as a plugin
ifeq ($(ENABLE_BAGEL), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
