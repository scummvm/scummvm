MODULE := scumm

SCUMM_OBJS = \
	scumm/actor.o \
	scumm/akos.o \
	scumm/base-costume.o \
	scumm/bomp.o \
	scumm/boxes.o \
	scumm/bundle.o \
	scumm/camera.o \
	scumm/charset.o \
	scumm/costume.o \
	scumm/debugger.o \
	scumm/dialogs.o \
	scumm/gfx.o \
	scumm/imuse.o \
	scumm/imuse_digi.o \
	scumm/imuse_player.o \
	scumm/instrument.o \
	scumm/help.o \
	scumm/midiparser_ro.o \
	scumm/nut_renderer.o \
	scumm/object.o \
	scumm/player_v1.o\
	scumm/player_v2.o\
	scumm/player_v3a.o\
	scumm/resource.o \
	scumm/resource_v2.o \
	scumm/resource_v3.o \
	scumm/resource_v4.o \
	scumm/saveload.o \
	scumm/script.o \
	scumm/script_v2.o \
	scumm/script_v5.o \
	scumm/script_v6.o \
	scumm/script_v8.o \
	scumm/scummvm.o \
	scumm/sound.o \
	scumm/string.o \
	scumm/usage_bits.o \
	scumm/vars.o \
	scumm/verbs.o

SMUSH_OBJS = \
	scumm/smush/chunk.o \
	scumm/smush/codec1.o \
	scumm/smush/codec37.o \
	scumm/smush/codec47.o \
	scumm/smush/imuse_channel.o \
	scumm/smush/smush_player.o \
	scumm/smush/saud_channel.o \
	scumm/smush/smush_mixer.o \
	scumm/smush/smush_font.o

MODULE_OBJS = $(SCUMM_OBJS) $(SMUSH_OBJS)

# Include common rules 
include common.rules
