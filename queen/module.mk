MODULE := queen

MODULE_OBJS = \
	queen/graphics.o \
	queen/logic.o \
	queen/resource.o \
	queen/restables.o \
	queen/queen.o \
	queen/version.o \

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include common.rules
