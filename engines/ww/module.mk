MODULE := engines/ww

MODULE_OBJS := \
	detection.o \
	gamelogic.o \
	gamemap.o \
	gxlarchive.o \
	graphics.o \
	helper.o \
	metaengine.o \
	pathfinding.o \
	sound.o \
	staticdata.o \
	ww.o \
	wwintro.o \
	wwintro_demo.o \
	wwintro_dh.o \
	wwintro_full.o
	

# This module can be built as a plugin
ifeq ($(ENABLE_WW), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
