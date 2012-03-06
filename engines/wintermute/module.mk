MODULE := engines/wintermute
 
MODULE_OBJS := \
	scriptables/ScScript.o \
	scriptables/ScStack.o \
	scriptables/ScValue.o \
	scriptables/SXArray.o \
	scriptables/SXDate.o \
	scriptables/SXMath.o \
	scriptables/SXMemBuffer.o \
	BBase.o \
	BNamedObject.o \
	BParser.o \
	BScriptable.o \
	BViewport.o \
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
