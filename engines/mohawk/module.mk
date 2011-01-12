MODULE := engines/mohawk

MODULE_OBJS = \
	bitmap.o \
	console.o \
	cursors.o \
	detection.o \
	dialogs.o \
	graphics.o \
	livingbooks.o \
	livingbooks_code.o \
	mohawk.o \
	myst.o \
	myst_areas.o \
	myst_vars.o \
	myst_scripts.o \
	myst_state.o \
	resource.o \
	resource_cache.o \
	riven.o \
	riven_external.o \
	riven_saveload.o \
	riven_scripts.o \
	riven_vars.o \
	sound.o \
	video.o \
	myst_stacks/channelwood.o \
	myst_stacks/credits.o \
	myst_stacks/demo.o \
	myst_stacks/dni.o \
	myst_stacks/intro.o \
	myst_stacks/makingof.o \
	myst_stacks/mechanical.o \
	myst_stacks/myst.o \
	myst_stacks/preview.o \
	myst_stacks/selenitic.o \
	myst_stacks/slides.o \
	myst_stacks/stoneship.o

# This module can be built as a plugin
ifeq ($(ENABLE_MOHAWK), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
