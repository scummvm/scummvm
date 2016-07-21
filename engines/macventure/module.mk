MODULE := engines/macventure

MODULE_OBJS := \
	image.o \
	detection.o \
	object.o \
	text.o \
	world.o \
	script.o \
	macventure.o \
	gui.o \
	dialog.o \
	controls.o \
	prebuilt_dialogs.o \
	windows.o


MODULE_DIRS += \
	engines/macventure

# This module can be built as a plugin
ifeq ($(ENABLE_MACVENTURE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
