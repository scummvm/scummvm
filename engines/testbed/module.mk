MODULE := engines/testbed
 
MODULE_OBJS := \
	detection.o \
	fs.o \
	graphics.o \
	savegame.o \
	testbed.o
 
MODULE_DIRS += \
	engines/testbed
 
# This module can be built as a plugin
ifeq ($(ENABLE_TESTBED), DYNAMIC_PLUGIN)
PLUGIN := 1
endif
 
# Include common rules 
include $(srcdir)/rules.mk
