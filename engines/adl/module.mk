MODULE := engines/adl

MODULE_OBJS := \
	detection.o \
	display.o \
	adl.o \
	adl_v1.o \
	parser.o

MODULE_DIRS += \
	engines/adl

# This module can be built as a plugin
ifeq ($(ENABLE_ADL), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
