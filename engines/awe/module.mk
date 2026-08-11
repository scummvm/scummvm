MODULE := engines/awe

MODULE_OBJS = \
	metaengine.o \
	awe.o \
	aifc_player.o \
	bitmap.o \
	engine.o \
	graphics_gl.o \
	graphics_soft.o \
	pak.o \
	resource.o \
	resource_3do.o \
	resource_nth.o \
	resource_win31.o \
	script.o \
	sfx_player.o \
	sound.o \
	static_res.o \
	system_stub.o \
	unpack.o \
	video.o

# This module can be built as a plugin
ifeq ($(ENABLE_AWE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
