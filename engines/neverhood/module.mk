MODULE := engines/neverhood

MODULE_OBJS = \
	blbarchive.o \
	detection.o \
	entity.o \
	gamemodule.o \
	graphics.o \
	module.o \
	neverhood.o \
	palette.o \
	resource.o \
	resourceman.o

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
