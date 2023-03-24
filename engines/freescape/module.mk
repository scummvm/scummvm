MODULE := engines/freescape

MODULE_OBJS := \
	area.o \
	assets.o \
	demo.o \
	freescape.o \
	games/castle.o \
	games/dark/dark.o \
	games/dark/dos.o \
	games/driller/amiga.o \
	games/driller/atari.o \
	games/driller/c64.o \
	games/driller/cpc.o \
	games/driller/dos.o \
	games/driller/driller.o \
	games/driller/zx.o \
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
	scr.o \
	sound.o \
	ui.o

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
