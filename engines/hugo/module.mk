MODULE := engines/hugo

MODULE_OBJS := \
	detection.o \
	display.o \
	engine.o \
	file.o \
	hugo.o \
	intro.o \
	inventory.o \
	mouse.o \
	parser.o \
	route.o \
	schedule.o \
	sound.o \
	util.o

# This module can be built as a plugin
ifeq ($(ENABLE_HUGO), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
