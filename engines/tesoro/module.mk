MODULE := engines/tesoro

MODULE_OBJS = \
	tesoro.o \
	console.o \
	metaengine.o \
	core/animimage.o \
	core/animbank.o \
	core/animset.o \
	core/archive.o \
	core/archiveindex.o \
	core/camera.o \
	core/cameraframe.o \
	core/clock.o \
	core/cursor.o \
	core/display.o \
	core/hovertext.o \
	core/level.o \
	core/namekey.o \
	core/object.o \
	core/renderqueue.o \
	core/room.o \
	core/spriteobject.o \
	core/textqueue.o

# This module can be built as a plugin
ifeq ($(ENABLE_TESORO), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
