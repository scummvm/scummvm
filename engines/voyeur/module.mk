MODULE := engines/voyeur

MODULE_OBJS := \
	detection.o \
	voyeur.o

# This module can be built as a plugin
ifeq ($(ENABLE_VOYEUR), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
