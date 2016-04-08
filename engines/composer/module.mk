MODULE := engines/composer

MODULE_OBJS = \
	console.o \
	composer.o \
	detection.o \
	graphics.o \
	resource.o \
	saveload.o \
	scripting.o

# This module can be built as a plugin
ifeq ($(ENABLE_COMPOSER), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
