MODULE := engines/darkseed

MODULE_OBJS = \
	darkseed.o \
	console.o \
	anm.o \
	img.o \
	nsp.o \
	pic.o \
	pal.o \
	titlefont.o \
	tostext.o \
	metaengine.o

# This module can be built as a plugin
ifeq ($(ENABLE_DARKSEED), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
