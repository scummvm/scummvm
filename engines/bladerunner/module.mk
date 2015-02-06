MODULE := engines/bladerunner

MODULE_OBJS = \
	ambient_sounds.o \
	archive.o \
	aud_decoder.o \
	audio_player.o \
	bladerunner.o \
	boundingbox.o \
	chapters.o \
	decompress_lcw.o \
	decompress_lzo.o \
	detection.o \
	gameflags.o \
	gameinfo.o \
	image.o \
	matrix.o \
	outtake.o \
	scene.o \
	script/rc01.o \
	script/script.o \
	set.o \
	settings.o \
	slice_animations.o \
	slice_renderer.o \
	view.o \
	vqa_decoder.o \
	vqa_player.o

# This module can be built as a plugin
ifeq ($(ENABLE_BLADERUNNER), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
