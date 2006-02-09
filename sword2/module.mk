MODULE := sword2

MODULE_OBJS := \
	sword2/_mouse.o \
	sword2/animation.o \
	sword2/anims.o \
	sword2/build_display.o \
	sword2/console.o \
	sword2/controls.o \
	sword2/d_draw.o \
	sword2/debug.o \
	sword2/events.o \
	sword2/function.o \
	sword2/icons.o \
	sword2/interpreter.o \
	sword2/layers.o \
	sword2/logic.o \
	sword2/maketext.o \
	sword2/memory.o \
	sword2/menu.o \
	sword2/mouse.o \
	sword2/music.o \
	sword2/palette.o \
	sword2/protocol.o \
	sword2/rdwin.o \
	sword2/render.o \
	sword2/resman.o \
	sword2/router.o \
	sword2/save_rest.o \
	sword2/scroll.o \
	sword2/sound.o \
	sword2/speech.o \
	sword2/sprite.o \
	sword2/startup.o \
	sword2/sword2.o \
	sword2/sync.o \
	sword2/walker.o

MODULE_DIRS += \
	sword2

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
