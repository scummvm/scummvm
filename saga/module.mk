MODULE := saga

MODULE_OBJS = \
	saga/saga.o \
	saga/binread.o \
	saga/gamedesc.o \
	saga/resfile.o

MODULE_DIRS += \
	saga

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
