MODULE := scumm

SCUMM_OBJS = \
	scumm/actor.o \
	scumm/akos.o \
	scumm/boxes.o \
	scumm/bundle.o \
	scumm/costume.o \
	scumm/debugger.o \
	scumm/dialogs.o \
	scumm/gfx.o \
	scumm/imuse.o \
	scumm/instrument.o \
	scumm/object.o \
	scumm/resource.o \
	scumm/resource_v2.o \
	scumm/resource_v3.o \
	scumm/resource_v4.o \
	scumm/saveload.o \
	scumm/script.o \
	scumm/script_v5.o \
	scumm/script_v6.o \
	scumm/script_v8.o \
	scumm/scummvm.o \
	scumm/sound.o \
	scumm/string.o \
	scumm/vars.o \
	scumm/verbs.o \

SMUSH_OBJS = \
	scumm/smush/blitter.o \
	scumm/smush/brenderer.o \
	scumm/smush/chunk.o \
	scumm/smush/codec1.o \
	scumm/smush/codec37.o \
	scumm/smush/codec44.o \
	scumm/smush/codec47.o \
	scumm/smush/color.o \
	scumm/smush/frenderer.o \
	scumm/smush/imuse_channel.o \
	scumm/smush/player.o \
	scumm/smush/saud_channel.o \
	scumm/smush/scumm_renderer.o

MODULE_OBJS = $(SCUMM_OBJS) $(SMUSH_OBJS)

# Include common rules 
include common.rules
