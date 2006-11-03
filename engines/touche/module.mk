MODULE := engines/touche

MODULE_OBJS := \
	graphics.o \
	plugin.o \
	opcodes.o \
	resource.o \
	saveload.o \
	staticres.o \
	touche.o \
	ui.o

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
