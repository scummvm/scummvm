MODULE := engines/tony

MODULE_OBJS := \
	detection.o \
	tony.o \
	utils.o \
	mpal/expr.o \
	mpal/loadmpc.o \
	mpal/mpal.o \
	mpal/lzo.o \
	mpal/stubs.o

# This module can be built as a plugin
ifeq ($(ENABLE_TONY), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
