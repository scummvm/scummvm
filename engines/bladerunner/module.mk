MODULE := engines/bladerunner

MODULE_OBJS = \
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
	decompress_lcw.o \
	decompress_lzo.o \
	detection.o \
	gameflags.o \
	gameinfo.o \
	image.o \
	matrix.o \
	outtake.o \
	scene.o \
	script/init.o \
	script/rc01.o \
	script/script.o \
	set.o \
	settings.o \
	slice_animations.o \
	slice_renderer.o \
	text_resource.o \
	view.o \
	vqa_decoder.o \
	vqa_player.o

# This module can be built as a plugin
ifeq ($(ENABLE_BLADERUNNER), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
