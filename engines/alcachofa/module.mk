MODULE := engines/alcachofa

MODULE_OBJS = \
	alcachofa.o \
	camera.o \
	common.o \
	console.o \
	game.o \
	game-movie-adventure.o \
	game-objects.o \
	general-objects.o \
	global-ui.o \
	graphics.o \
	graphics-opengl-base.o \
	input.o \
	menu.o \
	metaengine.o \
	player.o \
	rooms.o \
	scheduler.o \
	script.o \
	shape.o \
	sounds.o \
	ui-objects.o

ifdef USE_OPENGL_GAME
MODULE_OBJS += graphics-opengl.o
else # create_project cannot handle else and ifdef on the same line
ifdef USE_OPENGL_SHADERS
MODULE_OBJS += graphics-opengl.o
endif
endif

ifdef USE_OPENGL_GAME
MODULE_OBJS += \
	graphics-opengl-classic.o
endif

ifdef USE_OPENGL_SHADERS
MODULE_OBJS += \
	graphics-opengl-shaders.o
endif

ifdef USE_TINYGL
MODULE_OBJS += \
	graphics-tinygl.o
endif

# This module can be built as a plugin
ifeq ($(ENABLE_ALCACHOFA), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
