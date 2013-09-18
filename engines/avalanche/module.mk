MODULE := engines/avalanche

MODULE_OBJS = \
	avalanche.o \
	graphics.o \
	parser.o \
	avalot.o \
	console.o \
	detection.o \
	gyro.o \
	pingo.o \
	scrolls.o \
	lucerna.o \
	celer.o \
	sequence.o \
	timer.o \
	animation.o \
	acci.o \
	menu.o \
	closing.o \
	sound.o

# This module can be built as a plugin
ifeq ($(ENABLE_AVALANCHE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
