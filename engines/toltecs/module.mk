MODULE := engines/toltecs

MODULE_OBJS = \
	animation.o \
	console.o \
	menu.o \
	metaengine.o \
	microtiles.o \
	movie.o \
	music.o \
	palette.o \
	toltecs.o \
	render.o \
	resource.o \
	saveload.o \
	screen.o \
	script.o \
	segmap.o \
	sound.o \
	sprite.o


# This module can be built as a plugin
ifeq ($(ENABLE_TOLTECS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
