MODULE := engines/comfy

MODULE_OBJS = \
	actor.o \
	anim_file.o \
	comfy.o \
	comfy_keyboard_ui.o \
	console.o \
	debug.o \
	expr.o \
	game.o \
	input.o \
	keybits.o \
	metaengine.o \
	midi.o \
	midiplyr/midiplyr.o \
	palette.o \
	scene.o \
	script.o \
	script_vm.o \
	sound.o \
	timer.o \
	video.o

# This module can be built as a plugin
ifeq ($(ENABLE_COMFY), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
