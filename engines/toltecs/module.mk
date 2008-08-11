MODULE := engines/toltecs

MODULE_OBJS = \
	animation.o \
	detection.o \
	input.o \
	palette.o \
	toltecs.o \
	resource.o \
	saveload.o \
	screen.o \
	script.o \
	segmap.o


# This module can be built as a plugin
ifeq ($(ENABLE_TOLTECS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
