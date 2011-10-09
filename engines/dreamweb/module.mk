MODULE := engines/dreamweb

MODULE_OBJS := \
	backdrop.o \
	console.o \
	detection.o \
	dreamweb.o \
	dreamgen.o \
	object.o \
	pathfind.o \
	print.o \
	saveload.o \
	sprite.o \
	stubs.o \
	use.o \
	vgagrafx.o

# This module can be built as a plugin
ifeq ($(ENABLE_DREAMWEB), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
