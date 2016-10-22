MODULE := engines/cryo

MODULE_OBJS = \
	clerror.o \
	clhnm.o \
	clsoundgroup.o \
	clsoundraw.o \
	cryolib.o \
	sound.o \
	eden.o \
	cryo.o \
	detection.o \
	staticdata.o

# This module can be built as a plugin
ifeq ($(ENABLE_CRYO), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
