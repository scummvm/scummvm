MODULE := engines/wage

MODULE_OBJS := \
	combat.o \
	debugger.o \
	design.o \
	dialog.o \
	entities.o \
	gui.o \
	metaengine.o \
	randomhat.o \
	saveload.o \
	script.o \
	sound.o \
	util.o \
	wage.o \
	world.o

MODULE_DIRS += \
	engines/wage

# This module can be built as a plugin
ifeq ($(ENABLE_WAGE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
