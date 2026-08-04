MODULE := engines/ripper

MODULE_OBJS := \
	briefing.o \
	combat/combat.o \
	combat/resources.o \
	combat/mechini.o \
	console.o \
	controls.o \
	cursor.o \
	cyber.o \
	dialogue.o \
	display.o \
	iavf.o \
	iff.o \
	input.o \
	inventory.o \
	media/audio.o \
	media/display.o \
	media/plan.o \
	media/source.o \
	media/video.o \
	menu.o \
	metaengine.o \
	milestones.o \
	modal/presentation.o \
	modal_dialog.o \
	options_panel.o \
	puzzles/board_arrangement.o \
	puzzles/calculator.o \
	puzzles/cd_in_book.o \
	puzzles/circuit_chip.o \
	puzzles/clock.o \
	puzzles/crystal.o \
	puzzles/eight_button_sequence.o \
	puzzles/gc_csh.o \
	puzzles/ka_book_code.o \
	puzzles/kd_shooting_gallery.o \
	puzzles/keypad_sequence.o \
	puzzles/kj_blob_shooter.o \
	puzzles/rolodex.o \
	puzzles/shock_lever.o \
	puzzles/stained_glass.o \
	puzzles/table_gate.o \
	puzzles/web_grid_shift.o \
	remote_control.o \
	resources.o \
	ripper.o \
	saveload.o \
	scene_dispatcher.o \
	scene_audio.o \
	script/compiled_script.o \
	scenes/ebz2s_scene.o \
	scenes/gym_scene.o \
	scenes/librarian_scene.o \
	scenes/scene.o \
	scenes/tube_scene.o \
	scenes/wofford_scene.o \
	script.o \
	settings.o \
	toolbar.o \
	wac/broken_mug.o \
	wac/database.o \
	wac/database_catalog.o \
	wac/journal.o \
	wac/still_image.o \
	wac/voice_lock.o \
	wac/voice_lock_model.o \
	wac/voice_lock_renderer.o \
	wac/wac.o \
	world_map.o

# This module can be built as a plugin
ifeq ($(ENABLE_RIPPER), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
