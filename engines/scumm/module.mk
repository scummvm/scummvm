MODULE := engines/scumm

MODULE_OBJS := \
	engines/scumm/actor.o \
	engines/scumm/akos.o \
	engines/scumm/base-costume.o \
	engines/scumm/bomp.o \
	engines/scumm/boxes.o \
	engines/scumm/camera.o \
	engines/scumm/charset.o \
	engines/scumm/costume.o \
	engines/scumm/cursor.o \
	engines/scumm/debugger.o \
	engines/scumm/dialogs.o \
	engines/scumm/gfx.o \
	engines/scumm/imuse.o \
	engines/scumm/imuse_player.o \
	engines/scumm/input.o \
	engines/scumm/instrument.o \
	engines/scumm/help.o \
	engines/scumm/midiparser_ro.o \
	engines/scumm/midiparser_eup.o \
	engines/scumm/object.o \
	engines/scumm/palette.o \
	engines/scumm/player_mod.o \
	engines/scumm/player_v1.o \
	engines/scumm/player_nes.o \
	engines/scumm/player_v2.o \
	engines/scumm/player_v2a.o \
	engines/scumm/player_v3a.o \
	engines/scumm/resource.o \
	engines/scumm/resource_v2.o \
	engines/scumm/resource_v3.o \
	engines/scumm/resource_v4.o \
	engines/scumm/room.o \
	engines/scumm/saveload.o \
	engines/scumm/script.o \
	engines/scumm/script_c64.o \
	engines/scumm/script_v2.o \
	engines/scumm/script_v5.o \
	engines/scumm/script_v6.o \
	engines/scumm/script_v6he.o \
	engines/scumm/scumm.o \
	engines/scumm/sound.o \
	engines/scumm/sound_he.o \
	engines/scumm/string.o \
	engines/scumm/usage_bits.o \
	engines/scumm/util.o \
	engines/scumm/vars.o \
	engines/scumm/verbs.o \
	engines/scumm/thumbnail.o

ifndef DISABLE_SCUMM_7_8
MODULE_OBJS += \
	engines/scumm/nut_renderer.o \
	engines/scumm/script_v8.o \
	engines/scumm/imuse_digi/dimuse.o \
	engines/scumm/imuse_digi/dimuse_bndmgr.o \
	engines/scumm/imuse_digi/dimuse_codecs.o \
	engines/scumm/imuse_digi/dimuse_music.o \
	engines/scumm/imuse_digi/dimuse_sndmgr.o \
	engines/scumm/imuse_digi/dimuse_script.o \
	engines/scumm/imuse_digi/dimuse_track.o \
	engines/scumm/imuse_digi/dimuse_tables.o \
	engines/scumm/insane/insane.o \
	engines/scumm/insane/insane_ben.o \
	engines/scumm/insane/insane_enemy.o \
	engines/scumm/insane/insane_scenes.o \
	engines/scumm/insane/insane_iact.o \
	engines/scumm/smush/chunk.o \
	engines/scumm/smush/codec1.o \
	engines/scumm/smush/codec37.o \
	engines/scumm/smush/codec47.o \
	engines/scumm/smush/imuse_channel.o \
	engines/scumm/smush/smush_player.o \
	engines/scumm/smush/saud_channel.o \
	engines/scumm/smush/smush_mixer.o \
	engines/scumm/smush/smush_font.o
endif

ifndef DISABLE_HE
MODULE_OBJS += \
	engines/scumm/floodfill_he.o \
	engines/scumm/logic_he.o \
	engines/scumm/palette_he.o \
	engines/scumm/resource_v7he.o \
	engines/scumm/script_v7he.o \
	engines/scumm/script_v72he.o \
	engines/scumm/script_v80he.o \
	engines/scumm/script_v90he.o \
	engines/scumm/script_v100he.o \
	engines/scumm/sprite_he.o \
	engines/scumm/wiz_he.o
endif

MODULE_DIRS += \
	engines/scumm \
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
