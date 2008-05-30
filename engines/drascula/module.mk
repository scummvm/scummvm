MODULE := engines/drascula

MODULE_OBJS = \
	animation.o \
	detection.o \
	drascula.o \
	rooms.o \
	talk.o


# This module can be built as a plugin
ifeq ($(ENABLE_DRASCULA), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
