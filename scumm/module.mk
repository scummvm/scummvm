MODULE := scumm

MODULE_OBJS := \
	scumm/actor.o \
	scumm/akos.o \
	scumm/base-costume.o \
	scumm/bomp.o \
	scumm/boxes.o \
	scumm/camera.o \
	scumm/charset.o \
	scumm/costume.o \
	scumm/cursor.o \
	scumm/debugger.o \
	scumm/dialogs.o \
	scumm/gfx.o \
	scumm/imuse.o \
	scumm/imuse_player.o \
	scumm/input.o \
	scumm/instrument.o \
	scumm/help.o \
	scumm/midiparser_ro.o \
	scumm/midiparser_eup.o \
	scumm/object.o \
	scumm/palette.o \
	scumm/player_mod.o \
	scumm/player_v1.o \
	scumm/player_nes.o \
	scumm/player_v2.o \
	scumm/player_v2a.o \
	scumm/player_v3a.o \
	scumm/resource.o \
	scumm/resource_v2.o \
	scumm/resource_v3.o \
	scumm/resource_v4.o \
	scumm/room.o \
	scumm/saveload.o \
	scumm/script.o \
	scumm/script_c64.o \
	scumm/script_v2.o \
	scumm/script_v5.o \
	scumm/script_v6.o \
	scumm/script_v6he.o \
	scumm/scumm.o \
	scumm/sound.o \
	scumm/sound_he.o \
	scumm/string.o \
	scumm/usage_bits.o \
	scumm/util.o \
	scumm/vars.o \
	scumm/verbs.o \
	scumm/thumbnail.o

ifndef DISABLE_SCUMM_7_8
MODULE_OBJS += \
	scumm/nut_renderer.o \
	scumm/script_v8.o \
	scumm/imuse_digi/dimuse.o \
	scumm/imuse_digi/dimuse_bndmgr.o \
	scumm/imuse_digi/dimuse_codecs.o \
	scumm/imuse_digi/dimuse_music.o \
	scumm/imuse_digi/dimuse_sndmgr.o \
	scumm/imuse_digi/dimuse_script.o \
	scumm/imuse_digi/dimuse_track.o \
	scumm/imuse_digi/dimuse_tables.o \
	scumm/insane/insane.o \
	scumm/insane/insane_ben.o \
	scumm/insane/insane_enemy.o \
	scumm/insane/insane_scenes.o \
	scumm/insane/insane_iact.o \
	scumm/smush/chunk.o \
	scumm/smush/codec1.o \
	scumm/smush/codec37.o \
	scumm/smush/codec47.o \
	scumm/smush/imuse_channel.o \
	scumm/smush/smush_player.o \
	scumm/smush/saud_channel.o \
	scumm/smush/smush_mixer.o \
	scumm/smush/smush_font.o
endif

ifndef DISABLE_HE
MODULE_OBJS += \
	scumm/floodfill_he.o \
	scumm/logic_he.o \
	scumm/palette_he.o \
	scumm/resource_v7he.o \
	scumm/script_v7he.o \
	scumm/script_v72he.o \
	scumm/script_v80he.o \
	scumm/script_v90he.o \
	scumm/script_v100he.o \
	scumm/sprite_he.o \
	scumm/wiz_he.o
endif

MODULE_DIRS += \
	scumm \
	scumm/imuse_digi \
	scumm/insane \
	scumm/smush

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
# HACK HACK evil HACK HACK
PLUGIN_LDFLAGS += -lz
endif

# Include common rules 
include $(srcdir)/common.rules
