MODULE := engines/soltys
 
MODULE_OBJS := \
	console.o \
	detection.o \
	soltys.o
 
MODULE_DIRS += \
	engines/soltys
 
# This module can be built as a plugin
ifeq ($(ENABLE_SOLTYS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif
 
# Include common rules 
include $(srcdir)/rules.mk