MODULE := engines/dgds

MODULE_OBJS := \
	ads.o \
	clock.o \
	console.o \
	decompress.o \
	dgds.o \
	dgds_rect.o \
	dialog.o \
	font.o \
	game_palettes.o \
	globals.o \
	image.o \
	inventory.o \
	menu.o \
	metaengine.o \
	music.o \
	parser.o \
	request.o \
	resource.o \
	scripts.o \
	ttm.o \
	scene.o \
	sound.o

# This module can be built as a plugin
ifeq ($(ENABLE_DGDS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
