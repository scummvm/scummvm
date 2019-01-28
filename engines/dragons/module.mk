MODULE := engines/dragons
 
MODULE_OBJS := \
    actor.o \
    actorresource.o \
    background.o \
    bigfile.o \
	detection.o \
    dragonflg.o \
    dragonini.o \
    dragonobd.o \
    dragonrms.o \
    dragonvar.o \
	dragons.o \
	scene.o \
	screen.o \
	scriptopcodes.o \
	sequenceopcodes.o \
	specialopcodes.o

MODULE_DIRS += \
	engines/dragons
 
# This module can be built as a plugin
ifeq ($(ENABLE_QUUX), DYNAMIC_PLUGIN)
PLUGIN := 1
endif
 
# Include common rules 
include $(srcdir)/rules.mk
