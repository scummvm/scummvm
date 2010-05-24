MODULE := engines/mohawk

MODULE_OBJS = \
	bitmap.o \
	console.o \
	detection.o \
	dialogs.o \
	graphics.o \
	livingbooks.o \
	mohawk.o \
	myst.o \
	myst_vars.o \
	myst_saveload.o \
	myst_scripts.o \
	resource.o \
	resource_cache.o \
	riven.o \
	riven_external.o \
	riven_saveload.o \
	riven_scripts.o \
	riven_vars.o \
	sound.o \
	video.o


# This module can be built as a plugin
ifeq ($(ENABLE_MOHAWK), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
