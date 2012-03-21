MODULE := engines/mortevielle

MODULE_OBJS := \
	actions.o \
	detection.o \
	dialogs.o \
	graphics.o \
	keyboard.o \
	menu.o \
	mor.o \
	mortevielle.o \
	mouse.o \
	outtext.o \
	saveload.o \
	sound.o \
	speech.o \
	var_mor.o

# This module can be built as a plugin
ifeq ($(ENABLE_MORTEVIELLE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/rules.mk
