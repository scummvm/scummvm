MODULE := engines/agi

MODULE_OBJS := \
	agi.o \
	checks.o \
	console.o \
	cycle.o \
	detection.o \
	global.o \
	graphics.o \
	id.o \
	inv.o \
	keyboard.o \
	loader_v2.o \
	loader_v3.o \
	logic.o \
	lzw.o \
	menu.o \
	motion.o \
	objects.o \
	op_cmd.o \
	op_dbg.o \
	op_test.o \
	picture.o \
	preagi.o \
	preagi_common.o \
	preagi_mickey.o \
	preagi_troll.o \
	preagi_winnie.o \
	predictive.o \
	saveload.o \
	sound.o \
	sprite.o \
	text.o \
	view.o \
	wagparser.o \
	words.o


# This module can be built as a plugin
ifeq ($(ENABLE_AGI), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
