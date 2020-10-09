MODULE := engines/cryomni3d

MODULE_OBJS = \
	fonts/cryoextfont.o \
	fonts/cryofont.o \
	image/codecs/hlz.o \
	image/hlz.o \
	video/hnm_decoder.o \
	cryomni3d.o \
	datstream.o \
	dialogs_manager.o \
	fixed_image.o \
	font_manager.o \
	metaengine.o \
	mouse_boxes.o \
	objects.o \
	omni3d.o \
	sprites.o \
	wam_parser.o

ifdef ENABLE_VERSAILLES
MODULE_OBJS += \
	versailles/data.o \
	versailles/dialogs_manager.o \
	versailles/dialogs.o \
	versailles/documentation.o \
	versailles/engine.o \
	versailles/logic.o \
	versailles/menus.o \
	versailles/music.o \
	versailles/saveload.o \
	versailles/toolbar.o
endif

# This module can be built as a plugin
ifeq ($(ENABLE_CRYOMNI3D), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
