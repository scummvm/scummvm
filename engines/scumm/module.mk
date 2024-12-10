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
	file_engine.o \
	file_nes.o \
	gfx_gui.o \
	gfx_mac.o \
	gfx_towns.o \
	gfx.o \
	he/mixer_he.o \
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
	imuse/drivers/midi.o \
	imuse/drivers/macintosh.o \
	imuse/drivers/pcspk.o \
	input.o \
	ks_check.o \
	macgui/macgui.o \
	macgui/macgui_dialogwindow.o \
	macgui/macgui_impl.o \
	macgui/macgui_indy3.o \
	macgui/macgui_loom.o \
	macgui/macgui_strings.o \
	macgui/macgui_v5.o \
	macgui/macgui_v6.o \
	macgui/macgui_widgets.o \
	metaengine.o \
	midiparser_ro.o \
	object.o \
	palette.o \
	players/player_ad.o \
	players/player_apple2.o \
	players/player_he.o \
	players/player_mac_indy3.o \
	players/player_mac_loom_monkey.o \
	players/player_mac_new.o \
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
	players/player_v4a.o \
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
	soundcd.o \
	soundse.o \
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
	string_v7.o \
	script_v8.o \
	imuse_digi/dimuse_bndmgr.o \
	imuse_digi/dimuse_codecs.o \
	imuse_digi/dimuse_sndmgr.o \
	imuse_digi/dimuse_tables.o \
	imuse_digi/dimuse_engine.o \
	imuse_digi/dimuse_cmds.o \
	imuse_digi/dimuse_dispatch.o \
	imuse_digi/dimuse_fades.o \
	imuse_digi/dimuse_files.o \
	imuse_digi/dimuse_groups.o \
	imuse_digi/dimuse_internalmixer.o \
	imuse_digi/dimuse_scripts.o \
	imuse_digi/dimuse_streamer.o \
	imuse_digi/dimuse_tracks.o \
	imuse_digi/dimuse_triggers.o \
	imuse_digi/dimuse_utils.o \
	imuse_digi/dimuse_wave.o \
	imuse_digi/dimuse_waveout.o \
	insane/insane.o \
	insane/insane_ben.o \
	insane/insane_enemy.o \
	insane/insane_scenes.o \
	insane/insane_iact.o \
	smush/codec1.o \
	smush/codec20.o \
	smush/codec37.o \
	smush/codec47.o \
	smush/smush_player.o

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
	he/basketball/ai.o \
	he/basketball/basketball.o \
	he/basketball/collision/bball_collision.o \
	he/basketball/collision/bball_collision_basketball.o \
	he/basketball/collision/bball_collision_box.o \
	he/basketball/collision/bball_collision_cylinder.o \
	he/basketball/collision/bball_collision_node.o \
	he/basketball/collision/bball_collision_object.o \
	he/basketball/collision/bball_collision_player.o \
	he/basketball/collision/bball_collision_shields.o \
	he/basketball/collision/bball_collision_sphere.o \
	he/basketball/collision/bball_collision_stack.o \
	he/basketball/collision/bball_collision_tree.o \
	he/basketball/court.o \
	he/basketball/cursor.o \
	he/basketball/geo_translation.o \
	he/basketball/obstacle_avoidance.o \
	he/basketball/passing.o \
	he/basketball/shooting.o \
	he/basketball/trajectory.o \
	he/cup_player_he.o \
	he/gfx_comp/aux_comp.o \
	he/gfx_comp/mrle_comp.o \
	he/gfx_comp/trle_comp.o \
	he/gfx_primitives_he.o \
	he/logic_he.o \
	he/math_he.o \
	he/palette_he.o \
	he/polygon_he.o \
	he/script_v71he.o \
	he/script_v72he.o \
	he/script_v80he.o \
	he/script_v90he.o \
	he/script_v100he.o \
	he/sprite_he.o \
	he/wiz_he.o \
	he/wizwarp_he.o \
	he/localizer.o \
	he/logic/baseball2001.o \
	he/logic/basketball_logic.o \
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
	he/moonbase/dialog-mapgenerator.o \
	he/moonbase/map_katton.o \
	he/moonbase/map_main.o \
	he/moonbase/map_mif.o \
	he/moonbase/map_spiff.o \
	he/moonbase/moonbase.o \
	he/moonbase/moonbase_fow.o \
	he/moonbase/moonbase_gfx.o

ifdef USE_ENET
MODULE_OBJS += \
	dialog-createsession.o \
	dialog-sessionselector.o \
	he/net/net_main.o

ifdef USE_LIBCURL
MODULE_OBJS += \
	he/net/net_lobby.o
endif

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
