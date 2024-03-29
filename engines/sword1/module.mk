MODULE := engines/sword1

MODULE_OBJS := \
	animation.o \
	console.o \
	control.o \
	debug.o \
	eventman.o \
	logic.o \
	memman.o \
	menu.o \
	metaengine.o \
	mouse.o \
	objectman.o \
	resman.o \
	router.o \
	screen.o \
	sound.o \
	staticres.o \
	sword1.o \
	text.o

# This module can be built as a plugin
ifeq ($(ENABLE_SWORD1), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
