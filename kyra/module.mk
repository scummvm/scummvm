MODULE := kyra

MODULE_OBJS = \
	kyra/kyra.o

MODULE_DIRS += \
	kyra

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
