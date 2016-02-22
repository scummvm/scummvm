MODULE := engines/wage

MODULE_OBJS := \
	combat.o \
	design.o \
	detection.o \
	dialog.o \
	entities.o \
	gui.o \
	gui-console.o \
	menu.o \
	randomhat.o \
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
