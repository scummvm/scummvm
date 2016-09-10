MODULE := engines/bladerunner

MODULE_OBJS = \
	actor.o \
	actor_clues.o \
	actor_combat.o \
	actor_walk.o \
	adpcm_decoder.o \
	ambient_sounds.o \
	archive.o \
	aud_stream.o \
	audio_player.o \
	audio_speech.o \
	bladerunner.o \
	boundingbox.o \
	chapters.o \
	clues.o \
	combat.o \
	decompress_lcw.o \
	decompress_lzo.o \
	detection.o \
	fog.o \
	gameflags.o \
	gameinfo.o \
	image.o \
	item.o \
	items.o \
	light.o \
	lights.o \
	matrix.o \
	mouse.o \
	movement_track.o \
	outtake.o \
	regions.o \
	scene.o \
	scene_objects.o \
	script/ai_00_mccoy.o \
	script/aiscript_officer_leroy.o \
	script/init.o \
	script/rc01.o \
	script/script.o \
	set.o \
	settings.o \
	set_effects.o \
	shape.o \
	slice_animations.o \
	slice_renderer.o \
	text_resource.o \
	view.o \
	vqa_decoder.o \
	vqa_player.o \
	waypoints.o

# This module can be built as a plugin
ifeq ($(ENABLE_BLADERUNNER), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
