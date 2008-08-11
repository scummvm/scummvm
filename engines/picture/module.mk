MODULE := engines/picture

MODULE_OBJS = \
	animation.o \
	detection.o \
	input.o \
	palette.o \
	picture.o \
	resource.o \
	saveload.o \
	screen.o \
	script.o \
	segmap.o


# This module can be built as a plugin
ifeq ($(ENABLE_PICTURE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
