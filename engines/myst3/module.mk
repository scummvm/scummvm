MODULE := engines/myst3

MODULE_OBJS := \
	archive.o \
	detection.o \
	directoryentry.o \
	directorysubentry.o \
	myst3.o

# This module can be built as a plugin
ifeq ($(ENABLE_MYST3), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
