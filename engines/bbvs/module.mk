MODULE := engines/bbvs

MODULE_OBJS := \
	bbvs.o \
	dialogs.o \
	gamemodule.o \
	graphics.o \
	logic.o \
	metaengine.o \
	saveload.o \
	scene.o \
	sound.o \
	spritemodule.o \
	videoplayer.o \
	walk.o \
	minigames/bbairguitar.o \
	minigames/bbairguitar_anims.o \
	minigames/bbant.o \
	minigames/bbant_anims.o \
	minigames/bbloogie.o \
	minigames/bbloogie_anims.o \
	minigames/bbtennis.o \
	minigames/bbtennis_anims.o \
	minigames/minigame.o

# This module can be built as a plugin
ifeq ($(ENABLE_BBVS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
