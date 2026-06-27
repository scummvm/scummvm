MODULE := engines/hollywood

MODULE_OBJS = \
	hollywood.o \
	intro.o \
	metaengine.o \
	music.o \
	post_intro.o \
	resource.o

# This module can be built as a plugin
ifeq ($(ENABLE_HOLLYWOOD), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
