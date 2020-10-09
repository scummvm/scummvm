MODULE := engines/griffon

MODULE_OBJS := \
	combat.o \
	console.o \
	cutscenes.o \
	dialogs.o \
	draw.o \
	engine.o \
	gfx.o \
	griffon.o \
	input.o \
	logic.o \
	metaengine.o \
	resources.o \
	saveload.o \
	sound.o

MODULE_DIRS += \
	engines/griffon

# This module can be built as a plugin
ifeq ($(ENABLE_GRIFFON), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
