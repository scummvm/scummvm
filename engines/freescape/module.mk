MODULE := engines/freescape

MODULE_OBJS := \
	area.o \
	assets.o \
	debugger.o \
	demo.o \
	doodle.o \
	events.o \
	font.o \
	freescape.o \
	games/castle/castle.o \
	games/castle/amiga.o \
	games/castle/c64.o \
	games/castle/cpc.o \
	games/castle/dos.o \
	games/castle/zx.o \
	games/dark/amiga.o \
	games/dark/atari.o \
	games/dark/c64.o \
	games/dark/c64.music.o \
	games/dark/c64.sfx.o \
	games/dark/cpc.o \
	games/dark/dark.o \
	games/dark/dos.o \
	games/dark/zx.o \
	games/driller/amiga.o \
	games/driller/atari.o \
	games/driller/c64.o \
	games/driller/c64.music.o \
	games/driller/c64.sfx.o \
	games/driller/cpc.o \
	games/driller/dos.o \
	games/driller/driller.o \
	games/driller/sounds.o \
	games/driller/zx.o \
	games/eclipse/amiga.o \
	games/eclipse/atari.o \
	games/eclipse/atari.music.o \
	games/eclipse/c64.o \
	games/eclipse/c64.music.o \
	games/eclipse/ay.music.o \
	games/eclipse/c64.sfx.o \
	games/eclipse/dos.o \
	games/eclipse/eclipse.o \
	games/eclipse/opl.music.o \
	games/eclipse/cpc.o \
	games/eclipse/zx.o \
	games/palettes.o \
	gfx.o \
	loaders/8bitImage.o \
	loaders/8bitBinaryLoader.o \
	loaders/c64.o \
	language/8bitDetokeniser.o \
	language/instruction.o \
	metaengine.o \
	movement.o \
	objects/geometricobject.o \
	objects/group.o \
	objects/sensor.o \
	sweepAABB.o \
	sound/amiga.o \
	sound/common.o \
	sound/cpc.o \
	sound/dos.o \
	sound/zx.o \
	ui.o \
	unpack.o \
	wb.o

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
