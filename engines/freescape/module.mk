MODULE := engines/freescape

MODULE_OBJS := \
	area.o \
	demo.o \
	freescape.o \
	games/castle.o \
	games/dark.o \
	games/driller.o \
	games/eclipse.o \
	games/palettes.o \
	gfx.o \
	loaders/8bitBinaryLoader.o \
	language/8bitDetokeniser.o \
	language/instruction.o \
	metaengine.o \
	movement.o \
	neo.o \
	objects/geometricobject.o \
	objects/sensor.o \
	sound.o

ifdef USE_TINYGL
MODULE_OBJS += \
	gfx_tinygl.o \
	gfx_tinygl_texture.o
endif

ifdef USE_OPENGL
MODULE_OBJS += \
	gfx_opengl.o \
	gfx_opengl_texture.o
endif


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
