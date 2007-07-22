MODULE := engines/drascula

MODULE_OBJS = \
	detection.o \
	drascula.o


# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
