MODULE := engines/asylum

MODULE_OBJS := \
	bundles/bundle.o \
	bundles/graphicbundle.o \
	resources/resource.o \
	resources/graphic.o \
	resources/palette.o \
	asylum.o \
	detection.o \
	resourcemanager.o \
	screen.o \
	menu.o
	

# This module can be built as a plugin
ifeq ($(ENABLE_ASYLUM), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
