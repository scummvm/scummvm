MODULE := engines/playground3d

MODULE_OBJS := \
	metaengine.o \
	gfx.o \
	gfx_opengl.o \
	gfx_opengl_shaders.o \
	playground3d.o

ifdef USE_TINYGL
MODULE_OBJS += \
	gfx_tinygl.o
endif

MODULE_DIRS += \
	engines/playground3d

# This module can be built as a plugin
ifeq ($(ENABLE_PLAYGROUND3D), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
