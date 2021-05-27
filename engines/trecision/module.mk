MODULE := engines/trecision

MODULE_OBJS = \
	console.o \
	actor.o \
	anim.o \
	dialog.o \
	fastfile.o \
	graphics.o \
	inventory.o \
	logic.o \
	metaengine.o \
	resource.o \
	sound.o \
	saveload.o \
	scheduler.o \
	script.o \
	struct.o \
	text.o \
	trecision.o \
	utils.o \
	3d.o \
	video.o

# This module can be built as a plugin
ifeq ($(ENABLE_TRECISION), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
