MODULE := engines/wintermute
 
MODULE_OBJS := \
	BBase.o \
	BParser.o \
	detection.o \
	SysClass.o \
	SysInstance.o \
	wintermute.o
 
MODULE_DIRS += \
	engines/wintermute
 
# This module can be built as a plugin
ifeq ($(ENABLE_WINTERMUTE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif
 
# Include common rules 
include $(srcdir)/rules.mk
