MODULE := bs2

MODULE_OBJS := \
	bs2/anims.o \
	bs2/build_display.o \
	bs2/console.o \
	bs2/controls.o \
	bs2/debug.o \
	bs2/events.o \
	bs2/function.o \
	bs2/icons.o \
	bs2/interpreter.o \
	bs2/layers.o \
	bs2/logic.o \
	bs2/maketext.o \
	bs2/memory.o \
	bs2/mem_view.o \
	bs2/mouse.o \
	bs2/protocol.o \
	bs2/resman.o \
	bs2/router.o \
	bs2/save_rest.o \
	bs2/scroll.o \
	bs2/sound.o \
	bs2/speech.o \
	bs2/startup.o \
	bs2/sword2.o \
	bs2/sync.o \
	bs2/tony_gsdk.o \
	bs2/walker.o \
	bs2/driver/_console.o \
	bs2/driver/d_draw.o \
	bs2/driver/d_sound.o \
	bs2/driver/keyboard.o \
	bs2/driver/language.o \
	bs2/driver/menu.o \
	bs2/driver/misc.o \
	bs2/driver/_mouse.o \
	bs2/driver/palette.o \
	bs2/driver/rdwin.o \
	bs2/driver/render.o \
	bs2/driver/sprite.o

MODULE_DIRS += \
	bs2 \
	bs2/driver

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include common.rules
