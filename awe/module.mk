MODULE := awe

MODULE_OBJS = \
	awe/awe.o \
	awe/bank.o \
	awe/engine.o \
	awe/file.o \
	awe/logic.o \
	awe/resource.o \
	awe/sdlstub.o \
	awe/serializer.o \
	awe/staticres.o \
	awe/util.o \
	awe/video.o

MODULE_DIRS += \
	awe

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
