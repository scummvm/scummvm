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
	scumm/instrument.o \
	scumm/help.o \
	scumm/midiparser_ro.o \
	scumm/midiparser_eup.o \
	scumm/nut_renderer.o \
	scumm/object.o \
	scumm/palette.o \
	scumm/player_mod.o \
	scumm/player_v1.o \
	scumm/player_v2.o \
	scumm/player_v2a.o \
	scumm/player_v3a.o \
	scumm/resource.o \
	scumm/resource_v2.o \
	scumm/resource_v3.o \
	scumm/resource_v4.o \
	scumm/saveload.o \
	scumm/script.o \
	scumm/script_v2.o \
	scumm/script_v5.o \
	scumm/script_v6.o \
	scumm/script_v6he.o \
	scumm/script_v8.o \
	scumm/scummvm.o \
	scumm/sound.o \
	scumm/string.o \
	scumm/usage_bits.o \
	scumm/vars.o \
	scumm/verbs.o \
	scumm/imuse_digi/dimuse.o \
	scumm/imuse_digi/dimuse_bndmgr.o \
	scumm/imuse_digi/dimuse_codecs.o \
	scumm/imuse_digi/dimuse_music.o \
	scumm/imuse_digi/dimuse_sndmgr.o \
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

MODULE_DIRS += \
	scumm \
	scumm/imuse_digi \
	scumm/insane \
	scumm/smush

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
