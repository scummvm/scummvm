MODULE := engines/agi

MODULE_OBJS = \
	agi.o \
	agi_v2.o \
	agi_v3.o \
	checks.o \
	console.o \
	cycle.o \
	font.o \
	global.o \
	graphics.o \
	id.o \
	inv.o \
	keyboard.o \
	logic.o \
	lzw.o \
	menu.o \
	motion.o \
	objects.o \
	op_cmd.o \
	op_dbg.o \
	op_test.o \
	patches.o \
	picture.o \
	savegame.o \
	sound.o \
	sprite.o \
	text.o \
	view.o \
	words.o

MODULE_DIRS += \
	engines/agi

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
