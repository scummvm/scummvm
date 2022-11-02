MODULE := engines/cge

MODULE_OBJS := \
	bitmap.o \
	cge.o \
	cge_main.o \
	console.o \
	events.o \
	fileio.o \
	game.o \
	metaengine.o \
	snail.o \
	sound.o \
	talk.o \
	text.o \
	vga13h.o \
	vmenu.o \
	walk.o

MODULE_DIRS += \
	engines/cge

# This module can be built as a plugin
ifeq ($(ENABLE_CGE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o

# Skip building the following objects if a static
# module is enabled, because it already has the contents.
ifneq ($(ENABLE_CGE), STATIC_PLUGIN)
# External dependencies for detection.
DETECT_OBJS += $(MODULE)/fileio.o
endif
