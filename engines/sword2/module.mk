MODULE := engines/sword2

MODULE_OBJS := \
	engines/sword2/_mouse.o \
	engines/sword2/animation.o \
	engines/sword2/anims.o \
	engines/sword2/build_display.o \
	engines/sword2/console.o \
	engines/sword2/controls.o \
	engines/sword2/d_draw.o \
	engines/sword2/debug.o \
	engines/sword2/events.o \
	engines/sword2/function.o \
	engines/sword2/icons.o \
	engines/sword2/interpreter.o \
	engines/sword2/layers.o \
	engines/sword2/logic.o \
	engines/sword2/maketext.o \
	engines/sword2/memory.o \
	engines/sword2/menu.o \
	engines/sword2/mouse.o \
	engines/sword2/music.o \
	engines/sword2/palette.o \
	engines/sword2/protocol.o \
	engines/sword2/rdwin.o \
	engines/sword2/render.o \
	engines/sword2/resman.o \
	engines/sword2/router.o \
	engines/sword2/save_rest.o \
	engines/sword2/scroll.o \
	engines/sword2/sound.o \
	engines/sword2/speech.o \
	engines/sword2/sprite.o \
	engines/sword2/startup.o \
	engines/sword2/sword2.o \
	engines/sword2/sync.o \
	engines/sword2/walker.o

MODULE_DIRS += \
	engines/sword2

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
