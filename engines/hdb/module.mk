MODULE := engines/hdb

MODULE_OBJS := \
	detection.o \
	hdb.o

# This module can be built as a plugin
ifeq ($(ENABLE_HDB), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk