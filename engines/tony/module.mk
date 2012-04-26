MODULE := engines/tony

MODULE_OBJS := \
	detection.o \
	tony.o \
	lzo\stubs.o \
	mpal\expr.o \
	mpal\loadmpc.o \
	mpal\mpal.o \
	mpal\stubs.o

# This module can be built as a plugin
ifeq ($(ENABLE_TONY), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
