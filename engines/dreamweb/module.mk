MODULE := engines/dreamweb

MODULE_OBJS := \
	console.o \
	detection.o \
	dreamweb.o \
	dreamgen.o \
	stubs.o

# This module can be built as a plugin
ifeq ($(ENABLE_DREAMWEB), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
