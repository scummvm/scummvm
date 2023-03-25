MODULE := engines/crab

MODULE_OBJS = \
	crab.o \
	console.o \
	metaengine.o \
	filesystem.o \
	XMLDoc.o \
	loaders.o \
	ScreenSettings.o \
	GameParam.o \
	image/Image.o \
	image/ImageManager.o \
	input/cursor.o \
	timer.o \
	app.o \
	splash.o

# This module can be built as a plugin
ifeq ($(ENABLE_CRAB), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
