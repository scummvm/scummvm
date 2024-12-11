MODULE := engines/freescape

MODULE_OBJS := \
	area.o \
	assets.o \
	font.o \
	events.o \
	demo.o \
	freescape.o \
	games/castle/castle.o \
	games/castle/amiga.o \
	games/castle/cpc.o \
	games/castle/dos.o \
	games/castle/zx.o \
	games/dark/amiga.o \
	games/dark/atari.o \
	games/dark/cpc.o \
	games/dark/dark.o \
	games/dark/dos.o \
	games/dark/zx.o \
	games/driller/amiga.o \
	games/driller/atari.o \
	games/driller/c64.o \
	games/driller/cpc.o \
	games/driller/dos.o \
	games/driller/driller.o \
	games/driller/zx.o \
	games/eclipse/atari.o \
	games/eclipse/dos.o \
	games/eclipse/eclipse.o \
	games/eclipse/cpc.o \
	games/eclipse/zx.o \
	games/palettes.o \
	gfx.o \
	loaders/8bitImage.o \
	loaders/8bitBinaryLoader.o \
	language/8bitDetokeniser.o \
	language/instruction.o \
	metaengine.o \
	movement.o \
	objects/geometricobject.o \
	objects/group.o \
	objects/sensor.o \
	sweepAABB.o \
	sound.o \
	ui.o \
	unpack.o

ifdef USE_TINYGL
MODULE_OBJS += \
	gfx_tinygl.o \
	gfx_tinygl_texture.o
endif

ifdef USE_OPENGL_SHADERS
MODULE_OBJS += \
	gfx_opengl_shaders.o
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
