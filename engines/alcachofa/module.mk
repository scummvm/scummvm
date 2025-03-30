MODULE := engines/alcachofa

MODULE_OBJS = \
	alcachofa.o \
	camera.cpp \
	common.cpp \
	console.o \
	game-objects.cpp \
	general-objects.cpp \
	global-ui.cpp \
	graphics.cpp \
	graphics-opengl.cpp \
	input.cpp \
	metaengine.o \
	player.cpp \
	rooms.cpp \
	scheduler.cpp \
	script.cpp \
	shape.cpp \
	sounds.cpp \
	ui-objects.cpp \


# This module can be built as a plugin
ifeq ($(ENABLE_ALCACHOFA), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
