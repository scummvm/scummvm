MODULE := engines/colony

MODULE_OBJS := \
	animation.o \
	battle.o \
	colony.o \
	debugger.o \
	gfx.o \
	interaction.o \
	intro.o \
	map.o \
	metaengine.o \
	movement.o \
	render.o \
	renderer_opengl.o \
	sound.o \
	think.o \
	ui.o

MODULE_DIRS += \
	engines/colony

# This module can be built as a plugin
ifeq ($(ENABLE_COLONY), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
