MODULE := engines/composer

MODULE_OBJS = \
	composer.o \
	detection.o \
	resource.o

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
