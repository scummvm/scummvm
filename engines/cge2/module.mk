MODULE := engines/cge2

MODULE_OBJS = \
	cge2.o \
	detection.o \
	fileio.o \
	vga13h.o \
	bitmap.o \
	sound.o \
	cge2_main.o \
	text.o \
	hero.o \
	snail.o \
	spare.o \
	talk.o \
	events.o \
	map.o \
	vmenu.o \
	saveload.o \
	toolbar.o \
	inventory.o \
	console.o

# This module can be built as a plugin
ifeq ($(ENABLE_CGE2), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
