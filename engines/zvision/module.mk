MODULE := engines/zvision
 
MODULE_OBJS := \
	detection.o \
	zvision.o \
	zork_avi_decoder.o \
	zork_raw.o
 
MODULE_DIRS += \
	engines/zvision
 
# This module can be built as a plugin
ifeq ($(ENABLE_ZVISION), DYNAMIC_PLUGIN)
PLUGIN := 1
endif
 
# Include common rules 
include $(srcdir)/rules.mk