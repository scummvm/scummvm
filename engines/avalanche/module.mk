MODULE := engines/avalanche

MODULE_OBJS = \
	animation.o \
	avalanche.o \
	avalot.o \
	background.o \
	closing.o \
	console.o \
	graphics.o \
	dropdown.o \
	parser.o \
	dialogs.o \
	sequence.o \
	sound.o \
	timer.o \
	metaengine.o \
	nim.o \
	clock.o \
	ghostroom.o \
	help.o \
	shootemup.o \
	mainmenu.o \
	highscore.o

# This module can be built as a plugin
ifeq ($(ENABLE_AVALANCHE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
