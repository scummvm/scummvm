MODULE := engines/avalanche

MODULE_OBJS = \
	animation.o \
	avalanche.o \
	avalot.o \
	background.o \
	clock.o \
	closing.o \
	console.o \
	dialogs.o \
	dropdown.o \
	ghostroom.o \
	graphics.o \
	help.o \
	highscore.o \
	intro.o \
	mainmenu.o \
	metaengine.o \
	nim.o \
	parser.o \
	sequence.o \
	shootemup.o \
	sound.o \
	timer.o \

# This module can be built as a plugin
ifeq ($(ENABLE_AVALANCHE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
