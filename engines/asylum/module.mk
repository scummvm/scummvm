MODULE := engines/asylum

MODULE_OBJS := \
	asylum.o \
	detection.o \
	resman.o \
	screen.o \
	bundles/bundle.o \
	bundles/graphicbundle.o \
	resources/resource.o \
	resources/graphic.o \
	resources/movie.o

# This module can be built as a plugin
ifeq ($(ENABLE_ASYLUM), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
