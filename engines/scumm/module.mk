MODULE := engines/scumm

MODULE_OBJS := \
	actor.o \
	akos.o \
	base-costume.o \
	bomp.o \
	boxes.o \
	camera.o \
	cdda.o \
	charset.o \
	charset-fontdata.o \
	costume.o \
	cursor.o \
	debugger.o \
	dialogs.o \
	file.o \
	file_nes.o \
	gfx_towns.o \
	gfx.o \
	he/resource_he.o \
	he/script_v60he.o \
	he/script_v70he.o \
	he/sound_he.o \
	help.o \
	imuse/imuse.o \
	imuse/imuse_part.o \
	imuse/imuse_player.o \
	imuse/instrument.o \
	imuse/sysex_samnmax.o \
	imuse/sysex_scumm.o \
	imuse/drivers/amiga.o \
	imuse/drivers/fmtowns.o \
	imuse/drivers/mac_m68k.o \
	imuse/drivers/pcspk.o \
	input.o \
	ks_check.o \
	metaengine.o \
	midiparser_ro.o \
	object.o \
	palette.o \
	players/player_ad.o \
	players/player_apple2.o \
	players/player_he.o \
	players/player_mac.o \
	players/player_mod.o \
	players/player_nes.o \
	players/player_pce.o \
	players/player_sid.o \
	players/player_towns.o \
	players/player_v1.o \
	players/player_v2.o \
	players/player_v2a.o \
	players/player_v2base.o \
	players/player_v2cms.o \
	players/player_v3a.o \
	players/player_v3m.o \
	players/player_v4a.o \
	players/player_v5m.o \
	resource_v2.o \
	resource_v3.o \
	resource_v4.o \
	resource.o \
	room.o \
	saveload.o \
	script_v0.o \
	script_v2.o \
	script_v3.o \
	script_v4.o \
	script_v5.o \
	script_v6.o \
	script.o \
	scumm.o \
	sound.o \
	string.o \
	usage_bits.o \
	util.o \
	vars.o \
	verbs.o

ifdef USE_ARM_COSTUME_ASM
MODULE_OBJS += \
	proc3ARM.o
endif

ifdef ENABLE_SCUMM_7_8
MODULE_OBJS += \
	nut_renderer.o \
	script_v8.o \
	imuse_digi/dimuse.o \
	imuse_digi/dimuse_bndmgr.o \
	imuse_digi/dimuse_codecs.o \
	imuse_digi/dimuse_music.o \
	imuse_digi/dimuse_sndmgr.o \
	imuse_digi/dimuse_script.o \
	imuse_digi/dimuse_track.o \
	imuse_digi/dimuse_tables.o \
	insane/insane.o \
	insane/insane_ben.o \
	insane/insane_enemy.o \
	insane/insane_scenes.o \
	insane/insane_iact.o \
	smush/channel.o \
	smush/codec1.o \
	smush/codec20.o \
	smush/codec37.o \
	smush/codec47.o \
	smush/imuse_channel.o \
	smush/smush_player.o \
	smush/saud_channel.o \
	smush/smush_mixer.o \
	smush/smush_font.o

ifdef USE_ARM_SMUSH_ASM
MODULE_OBJS += \
	smush/codec47ARM.o
endif

endif

ifdef USE_ARM_GFX_ASM
MODULE_OBJS += \
	gfxARM.o
endif

ifdef ENABLE_HE
MODULE_OBJS += \
	he/animation_he.o \
	he/cup_player_he.o \
	he/floodfill_he.o \
	he/logic_he.o \
	he/palette_he.o \
	he/script_v71he.o \
	he/script_v72he.o \
	he/script_v80he.o \
	he/script_v90he.o \
	he/script_v100he.o \
	he/sprite_he.o \
	he/wiz_he.o \
	he/localizer.o \
	he/logic/baseball2001.o \
	he/logic/basketball.o \
	he/logic/football.o \
	he/logic/funshop.o \
	he/logic/moonbase_logic.o \
	he/logic/puttrace.o \
	he/logic/soccer.o \
	he/moonbase/ai_defenseunit.o \
	he/moonbase/ai_main.o \
	he/moonbase/ai_node.o \
	he/moonbase/ai_targetacquisition.o \
	he/moonbase/ai_traveller.o \
	he/moonbase/ai_tree.o \
	he/moonbase/ai_types.o \
	he/moonbase/ai_weapon.o \
	he/moonbase/distortion.o \
	he/moonbase/moonbase.o \
	he/moonbase/moonbase_fow.o

ifdef USE_LIBCURL
MODULE_OBJS += \
	he/moonbase/net_main.o
endif
endif

# This module can be built as a plugin
ifeq ($(ENABLE_SCUMM), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o

# Skip building the following objects if a static
# module is enabled, because it already has the contents.
ifneq ($(ENABLE_SCUMM), STATIC_PLUGIN)
DETECT_OBJS += $(MODULE)/file.o
DETECT_OBJS += $(MODULE)/file_nes.o
endif
