MODULE := engines/kyra

MODULE_OBJS := \
	engine/items_lok.o \
	engine/items_v2.o \
	engine/items_hof.o \
	engine/items_mr.o \
	engine/kyra_v1.o \
	engine/kyra_lok.o \
	engine/kyra_v2.o \
	engine/kyra_hof.o \
	engine/kyra_mr.o \
	engine/scene_v1.o \
	engine/scene_lok.o \
	engine/scene_v2.o \
	engine/scene_hof.o \
	engine/scene_mr.o \
	engine/sprites.o \
	engine/timer.o \
	engine/timer_lok.o \
	engine/timer_hof.o \
	engine/timer_mr.o \
	engine/util.o \
	graphics/animator_lok.o \
	graphics/animator_v2.o \
	graphics/animator_hof.o \
	graphics/animator_mr.o \
	graphics/animator_tim.o \
	graphics/screen.o \
	graphics/screen_lok.o \
	graphics/screen_v2.o \
	graphics/screen_hof.o \
	graphics/screen_mr.o \
	graphics/vqa.o \
	graphics/wsamovie.o \
	gui/debugger.o \
	gui/gui.o \
	gui/gui_v1.o \
	gui/gui_lok.o \
	gui/gui_v2.o \
	gui/gui_hof.o \
	gui/gui_mr.o \
	gui/saveload.o \
	gui/saveload_lok.o \
	gui/saveload_hof.o \
	gui/saveload_mr.o \
	metaengine.o \
	resource/resource.o \
	resource/resource_intern.o \
	resource/staticres.o \
	script/script_v1.o \
	script/script_lok.o \
	script/script_v2.o \
	script/script_hof.o \
	script/script_mr.o \
	script/script.o \
	script/script_tim.o \
	sequence/seqplayer_lok.o \
	sequence/sequences_lok.o \
	sequence/sequences_v2.o \
	sequence/sequences_hof.o \
	sequence/sequences_mr.o \
	sound/sound_amiga_lok.o \
	sound/sound_digital_mr.o \
	sound/sound_pc_midi.o \
	sound/sound_pc_v1.o \
	sound/sound_pc98_lok.o \
	sound/sound_pc98_v2.o \
	sound/sound_towns_lok.o \
	sound/sound.o \
	sound/sound_lok.o \
	sound/drivers/adlib.o \
	sound/drivers/audstream.o \
	sound/drivers/pcspeaker_v2.o \
	text/text.o \
	text/text_lok.o \
	text/text_hof.o \
	text/text_mr.o

KYRARPG_COMMON_OBJ = \
	engine/kyra_rpg.o \
	engine/scene_rpg.o \
	engine/sprites_rpg.o \
	engine/timer_rpg.o \
	gui/gui_rpg.o \
	gui/saveload_rpg.o \
	resource/staticres_rpg.o \
	text/text_rpg.o

ifdef ENABLE_LOL
MODULE_OBJS += \
	$(KYRARPG_COMMON_OBJ) \
	engine/items_lol.o \
	engine/lol.o \
	engine/scene_lol.o \
	engine/sprites_lol.o \
	engine/timer_lol.o \
	graphics/screen_lol.o \
	gui/gui_lol.o \
	gui/saveload_lol.o \
	resource/staticres_lol.o \
	script/script_lol.o \
	sequence/sequences_lol.o \
	sound/sound_lol.o \
	text/text_lol.o
endif

ifdef ENABLE_EOB
ifndef ENABLE_LOL
MODULE_OBJS += \
	$(KYRARPG_COMMON_OBJ)
endif
MODULE_OBJS += \
	engine/chargen.o \
	engine/eobcommon.o \
	engine/eob.o \
	engine/darkmoon.o \
	engine/items_eob.o \
	engine/magic_eob.o \
	engine/scene_eob.o \
	engine/sprites_eob.o \
	engine/timer_eob.o \
	graphics/screen_eob.o \
	graphics/screen_eob_amiga.o \
	graphics/screen_eob_pc98.o \
	graphics/screen_eob_segacd.o \
	graphics/screen_eob_towns.o \
	gui/gui_eob.o \
	gui/gui_eob_segacd.o \
	gui/saveload_eob.o \
	resource/resource_segacd.o \
	resource/staticres_eob.o \
	script/script_eob.o \
	sequence/seqplayer_eob_segacd.o \
	sequence/sequences_eob.o \
	sequence/sequences_darkmoon.o \
	sound/sound_amiga_eob.o \
	sound/sound_pc98_eob.o \
	sound/sound_segacd_eob.o \
	sound/sound_towns_darkmoon.o \
	sound/drivers/audiomaster2.o \
	sound/drivers/mlalf98.o \
	sound/drivers/pcspeaker_v1.o \
	sound/drivers/segacd.o \
	text/text_eob_segacd.o
endif

# This module can be built as a plugin
ifeq ($(ENABLE_KYRA), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# HACK: Skip this when including the file for detection objects.
ifeq "$(USE_RULES)" "1"
ifeq ($(BACKEND), maemo)
# Ugly workaround, screen.cpp crashes gcc version 3.4.4 (CodeSourcery ARM 2005q3-2) with anything but -O3
$(MODULE)/graphics/screen.o: $(MODULE)/graphics/screen.cpp
	$(MKDIR) $(*D)/$(DEPDIR)
	$(CXX) -Wp,-MMD,"$(*D)/$(DEPDIR)/$(*F).d",-MQ,"$@",-MP $(CXXFLAGS) -O3 $(CPPFLAGS) -c $(<) -o $*.o
endif # BACKEND=MAEMO
endif # USE_RULES

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
