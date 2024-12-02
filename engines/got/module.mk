MODULE := engines/got

MODULE_OBJS = \
	got.o \
	console.o \
	events.o \
	messages.o \
	metaengine.o \
	vars.o \
	gfx/images.o \
	gfx/palette.o \
	utils/file.o \
	utils/res_archive.o \
	views/main_menu.o \
	views/view.o \
	views/story.o

# This module can be built as a plugin
ifeq ($(ENABLE_GOT), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
