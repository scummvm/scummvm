MODULE := engines/prince

MODULE_OBJS = \
	script.o \
	graphics.o \
	mhwanh.o \
	detection.o \
	font.o \
	prince.o

# This module can be built as a plugin
ifeq ($(ENABLE_PRINCE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
