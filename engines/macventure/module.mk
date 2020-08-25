MODULE := engines/macventure

MODULE_OBJS := \
	container.o \
	controls.o \
	cursor.o \
	datafiles.o \
	dialog.o \
	gui.o \
	image.o \
	macventure.o \
	metaengine.o \
	prebuilt_dialogs.o \
	saveload.o \
	script.o \
	sound.o \
	text.o \
	windows.o \
	world.o


MODULE_DIRS += \
	engines/macventure

# This module can be built as a plugin
ifeq ($(ENABLE_MACVENTURE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
