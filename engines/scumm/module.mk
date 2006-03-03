MODULE := engines/scumm

MODULE_OBJS := \
	actor.o \
	akos.o \
	base-costume.o \
	bomp.o \
	boxes.o \
	camera.o \
	charset.o \
	costume.o \
	cursor.o \
	debugger.o \
	dialogs.o \
	file.o \
	gfx.o \
	he/script_v60he.o \
	he/sound_he.o \
	help.o \
	imuse/imuse_player.o \
	imuse/imuse.o \
	imuse/instrument.o \
	input.o \
	midiparser_eup.o \
	midiparser_ro.o \
	object.o \
	palette.o \
	player_mod.o \
	player_nes.o \
	player_v1.o \
	player_v2.o \
	player_v2a.o \
	player_v3a.o \
	plugin.o \
	resource_v2.o \
	resource_v3.o \
	resource_v4.o \
	resource.o \
	room.o \
	saveload.o \
	script_c64.o \
	script_v2.o \
	script_v5.o \
	script_v6.o \
	script.o \
	scumm.o \
	sound.o \
	string.o \
	thumbnail.o \
	usage_bits.o \
	util.o \
	vars.o \
	verbs.o

ifndef DISABLE_SCUMM_7_8
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
	smush/chunk.o \
	smush/codec1.o \
	smush/codec37.o \
	smush/codec47.o \
	smush/imuse_channel.o \
	smush/smush_player.o \
	smush/saud_channel.o \
	smush/smush_mixer.o \
	smush/smush_font.o
endif

ifndef DISABLE_HE
MODULE_OBJS += \
	he/floodfill_he.o \
	he/logic_he.o \
	he/palette_he.o \
	he/resource_he.o \
	he/script_v70he.o \
	he/script_v72he.o \
	he/script_v80he.o \
	he/script_v90he.o \
	he/script_v100he.o \
	he/sprite_he.o \
	he/wiz_he.o
endif

MODULE_DIRS += \
	engines/scumm \
	engines/scumm/he \
	engines/scumm/imuse \
	engines/scumm/imuse_digi \
	engines/scumm/insane \
	engines/scumm/smush

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
# HACK HACK evil HACK HACK
PLUGIN_LDFLAGS += -lz
endif

# Include common rules 
include $(srcdir)/common.rules
