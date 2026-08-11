MODULE := engines/phoenixvr

MODULE_OBJS = \
	arn.o \
	bigf.o \
	commands.o \
	commands_v1.o \
	commands_v2.o \
	game_state.o \
	console.o \
	metaengine.o \
	pakf.o \
	phoenixvr.o \
	rectf.o \
	region_set.o \
	script.o \
	script_v1.o \
	script_v2.o \
	variables.o \
	vr.o

# This module can be built as a plugin
ifeq ($(ENABLE_PHOENIXVR), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
