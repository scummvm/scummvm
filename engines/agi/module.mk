MODULE := engines/agi

MODULE_OBJS := \
	agi.o \
	checks.o \
	console.o \
	cycle.o \
	disk_image.o \
	font.o \
	global.o \
	graphics.o \
	inv.o \
	keyboard.o \
	loader_a2.o \
	loader_v1.o \
	loader_v2.o \
	loader_v3.o \
	logic.o \
	lzw.o \
	menu.o \
	metaengine.o \
	motion.o \
	objects.o \
	opcodes.o \
	op_cmd.o \
	op_dbg.o \
	op_test.o \
	picture.o \
	saveload.o \
	sound.o \
	sound_2gs.o \
	sound_coco3.o \
	sound_midi.o \
	sound_pcjr.o \
	sound_sarien.o \
	sprite.o \
	systemui.o \
	text.o \
	view.o \
	words.o \
	preagi/preagi.o \
	preagi/mickey.o \
	preagi/troll.o \
	preagi/winnie.o

# This module can be built as a plugin
ifeq ($(ENABLE_AGI), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o

# External dependencies of detection.
# This is unneeded by the engine module itself,
# so separate it completely.
DETECT_OBJS += $(MODULE)/wagparser.o

# Skip building the following objects if a static
# module is enabled, because it already has the contents.
ifneq ($(ENABLE_AGI), STATIC_PLUGIN)
# External dependencies for detection.
DETECT_OBJS += $(MODULE)/disk_image.o
endif
