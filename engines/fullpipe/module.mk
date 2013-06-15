MODULE := engines/fullpipe

MODULE_OBJS = \
	detection.o \
	fullpipe.o \
	ngiarchive.o \
	stateloader.o \
	utils.o

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
