MODULE := engines/freescape

MODULE_OBJS := \
	metaengine.o \
	freescape.o \
	area.o \
	games/castle.o \
	games/driller.o \
	games/eclipse.o \
	gfx.o \
	gfx_tinygl.o \
	gfx_tinygl_texture.o \
	objects/object.o \
	objects/entrance.o \
	objects/geometricobject.o \
	objects/sensor.o \
	loaders/8bitBinaryLoader.o \
	language/8bitDetokeniser.o \
	loaders/16bitBinaryLoader.o \
	language/16bitDetokeniser.o \
	language/instruction.o \
	palettes.o

MODULE_DIRS += \
	engines/freescape


# This module can be built as a plugin
ifeq ($(ENABLE_FREESCAPE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
