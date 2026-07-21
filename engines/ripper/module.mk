MODULE := engines/ripper

MODULE_OBJS := \
	briefing.o \
	combat/combat.o \
	combat/mechini.o \
	controls.o \
	cursor.o \
	cyber.o \
	dialogue.o \
	display.o \
	iavf.o \
	iff.o \
	input.o \
	inventory.o \
	media.o \
	menu.o \
	metaengine.o \
	milestones.o \
	modal_dialog.o \
	options_panel.o \
	puzzles/broken_mug.o \
	puzzles/calculator.o \
	puzzles/clock.o \
	puzzles/crystal.o \
	puzzles/gc_csh.o \
	puzzles/ka_book_code.o \
	puzzles/rolodex.o \
	puzzles/table_gate.o \
	remote_control.o \
	resources.o \
	ripper.o \
	saveload.o \
	scene_dispatcher.o \
	scenes/librarian_scene.o \
	scenes/scene.o \
	scenes/tube_scene.o \
	script.o \
	settings.o \
	toolbar.o \
	wac.o \
	world_map.o

# This module can be built as a plugin
ifeq ($(ENABLE_RIPPER), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
