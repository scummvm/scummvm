MODULE := engines/dgds

MODULE_OBJS := \
	console.o \
	decompress.o \
	dgds.o \
	font.o \
	globals.o \
	image.o \
	menu.o \
	metaengine.o \
	music.o \
	parser.o \
	request.o \
	resource.o \
	scripts.o \
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
