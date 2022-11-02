MODULE := engines/mohawk

MODULE_OBJS = \
	bitmap.o \
	console.o \
	cursors.o \
	dialogs.o \
	graphics.o \
	installer_archive.o \
	livingbooks.o \
	livingbooks_code.o \
	livingbooks_graphics.o \
	livingbooks_lbx.o \
	metaengine.o \
	riven_metaengine.o \
	myst_metaengine.o \
	mohawk.o \
	resource.o \
	sound.o \
	video.o \
	view.o

ifdef ENABLE_CSTIME
MODULE_OBJS += \
	cstime.o \
	cstime_cases.o \
	cstime_game.o \
	cstime_graphics.o \
	cstime_ui.o \
	cstime_view.o
endif

ifdef ENABLE_MYST
MODULE_OBJS += \
	myst.o \
	myst_areas.o \
	myst_card.o \
	myst_graphics.o \
	myst_scripts.o \
	myst_sound.o \
	myst_state.o \
	resource_cache.o \
	myst_stacks/channelwood.o \
	myst_stacks/credits.o \
	myst_stacks/demo.o \
	myst_stacks/dni.o \
	myst_stacks/intro.o \
	myst_stacks/makingof.o \
	myst_stacks/mechanical.o \
	myst_stacks/menu.o \
	myst_stacks/myst.o \
	myst_stacks/preview.o \
	myst_stacks/selenitic.o \
	myst_stacks/slides.o \
	myst_stacks/stoneship.o
endif

ifdef ENABLE_RIVEN
MODULE_OBJS += \
	riven.o \
	riven_card.o \
	riven_graphics.o \
	riven_inventory.o \
	riven_saveload.o \
	riven_scripts.o \
	riven_sound.o \
	riven_stack.o \
	riven_vars.o \
	riven_video.o \
	riven_stacks/aspit.o \
	riven_stacks/bspit.o \
	riven_stacks/domespit.o \
	riven_stacks/gspit.o \
	riven_stacks/jspit.o \
	riven_stacks/ospit.o \
	riven_stacks/pspit.o \
	riven_stacks/rspit.o \
	riven_stacks/tspit.o
endif

# This module can be built as a plugin
ifeq ($(ENABLE_MOHAWK), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o

# Skip building the following objects if a static
# module is enabled, because it already has the contents.
ifneq ($(ENABLE_MOHAWK), STATIC_PLUGIN)
DETECT_OBJS += $(MODULE)/myst_metaengine.o
DETECT_OBJS += $(MODULE)/riven_metaengine.o
endif
