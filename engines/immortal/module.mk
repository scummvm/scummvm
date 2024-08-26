MODULE := engines/immortal

MODULE_OBJS = \
	bullet.o \
	compression.o \
	cycle.o \
	door.o \
	drawChr.o \
	flameSet.o \
	immortal.o \
	kernal.o \
	level.o \
	logic.o \
	metaengine.o \
	misc.o \
	room.o \
	sprites.o \
	story.o \
	utilities.o \
	univ.o

#	object.o \
#	monster.o \
#	motives.o

# This module can be built as a plugin
ifeq ($(ENABLE_IMMORTAL), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
