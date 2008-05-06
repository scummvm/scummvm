MODULE := engines/kyra

MODULE_OBJS := \
	animator_v1.o \
	animator_v2.o \
	animator_hof.o \
	animator_mr.o \
	debugger.o \
	detection.o \
	gui.o \
	gui_v1.o \
	gui_v2.o \
	gui_hof.o \
	gui_mr.o \
	items_v1.o \
	items_v2.o \
	items_hof.o \
	items_mr.o \
	kyra.o \
	kyra_v1.o \
	kyra_v2.o \
	kyra_hof.o \
	kyra_mr.o \
	resource.o \
	saveload.o \
	saveload_v1.o \
	saveload_hof.o \
	saveload_mr.o \
	scene.o \
	scene_v1.o \
	scene_v2.o \
	scene_hof.o \
	scene_mr.o \
	screen.o \
	screen_v1.o \
	screen_v2.o \
	screen_hof.o \
	screen_mr.o \
	script_v1.o \
	script_v2.o \
	script_hof.o \
	script_mr.o \
	script.o \
	script_tim.o \
	seqplayer.o \
	sequences_v1.o \
	sequences_v2.o \
	sequences_hof.o \
	sequences_mr.o \
	sound_adlib.o \
	sound_digital.o \
	sound_towns.o \
	sound.o \
	sound_v1.o \
	sprites.o \
	staticres.o \
	text.o \
	text_v1.o \
	text_hof.o \
	text_mr.o \
	timer.o \
	timer_v1.o \
	timer_hof.o \
	timer_mr.o \
	vqa.o \
	wsamovie.o

# This module can be built as a plugin
ifeq ($(ENABLE_KYRA), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
