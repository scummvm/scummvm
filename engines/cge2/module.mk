MODULE := engines/cge2

MODULE_OBJS = \
	cge2.o \
	vga13h.o \
	bitmap.o \
	fileio.o \
	sound.o \
	cge2_main.o \
	text.o \
	hero.o \
	snail.o \
	spare.o \
	talk.o \
	events.o \
	map.o \
	metaengine.o \
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

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o

# Skip building the following objects if a static
# module is enabled, because it already has the contents.
ifneq ($(ENABLE_CGE2), STATIC_PLUGIN)
# External dependencies for detection.
DETECT_OBJS += $(MODULE)/fileio.o
endif
