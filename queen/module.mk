MODULE := queen

MODULE_OBJS = \
	queen/cutaway.o \
	queen/graphics.o \
	queen/logic.o \
	queen/queen.o \
	queen/resource.o \
	queen/restables.o \
	queen/talk.o

MODULE_DIRS += \
	queen

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include common.rules
