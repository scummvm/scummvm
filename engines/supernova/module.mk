MODULE := engines/supernova

MODULE_OBJS := \
	console.o \
	graphics.o \
	metaengine.o \
	resman.o \
	room.o \
	supernova1/rooms.o \
	supernova2/rooms.o \
	screen.o \
	sound.o \
	game-manager.o \
	supernova1/state.o \
	supernova2/state.o \
	supernova.o

MODULE_DIRS += \
	engines/supernova

# This module can be built as a plugin
ifeq ($(ENABLE_SUPERNOVA), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
