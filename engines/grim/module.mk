MODULE := engines/grim

MODULE_OBJS := \
	costume/bitmap_component.o \
	costume/chore.o \
	costume/colormap_component.o \
	costume/component.o \
	costume/head.o \
	costume/keyframe_component.o \
	costume/lua_var_component.o \
	costume/main_model_component.o \
	costume/material_component.o \
	costume/mesh_component.o \
	costume/model_component.o \
	costume/sound_component.o \
	costume/sprite_component.o \
	costume/anim_component.o \
	emi/costume/emianim_component.o \
	emi/costume/emichore.o \
	emi/costume/emihead.o \
	emi/costume/emiluavar_component.o \
	emi/costume/emiluacode_component.o \
	emi/costume/emimesh_component.o \
	emi/costume/emiskel_component.o \
	emi/costume/emisprite_component.o \
	emi/costume/emitexi_component.o \
	emi/sound/aifftrack.o \
	emi/sound/mp3track.o \
	emi/sound/scxtrack.o \
	emi/sound/vimatrack.o \
	emi/sound/track.o \
	emi/sound/emisound.o \
	emi/sound/codecs/scx.o \
	emi/animationemi.o \
	emi/costumeemi.o \
	emi/emi.o \
	emi/modelemi.o \
	emi/skeleton.o \
	emi/poolsound.o \
	emi/layer.o \
	emi/lua_v2.o \
	emi/lua_v2_actor.o \
	emi/lua_v2_sound.o \
	emi/emi_registry.o \
	imuse/imuse.o \
	imuse/imuse_mcmp_mgr.o \
	imuse/imuse_music.o \
	imuse/imuse_script.o \
	imuse/imuse_sndmgr.o \
	imuse/imuse_tables.o \
	imuse/imuse_track.o \
	lua/lapi.o \
	lua/lauxlib.o \
	lua/lbuffer.o \
	lua/lbuiltin.o \
	lua/ldo.o \
	lua/lfunc.o \
	lua/lgc.o \
	lua/liolib.o \
	lua/llex.o \
	lua/lmathlib.o \
	lua/lmem.o \
	lua/lobject.o \
	lua/lrestore.o \
	lua/lsave.o \
	lua/lstate.o \
	lua/lstring.o \
	lua/lstrlib.o \
	lua/lstx.o \
	lua/ltable.o \
	lua/ltask.o \
	lua/ltm.o \
	lua/lundump.o \
	lua/lvm.o \
	lua/lzio.o \
	movie/codecs/codec48.o \
	movie/codecs/blocky8.o \
	movie/codecs/blocky16.o \
	movie/codecs/vima.o \
	movie/codecs/smush_decoder.o \
	movie/bink.o \
	movie/mpeg.o \
	movie/movie.o \
	movie/quicktime.o \
	movie/smush.o \
	remastered/commentary.o \
	remastered/hotspot.o \
	remastered/lua_remastered.o \
	remastered/overlay.o \
	update/packfile.o \
	update/mscab.o \
	update/lang_filter.o \
	update/update.o \
	actor.o \
	animation.o \
	bitmap.o \
	costume.o \
	color.o \
	colormap.o \
	debug.o \
	detection.o \
	font.o \
	gfx_base.o \
	gfx_tinygl.o \
	gfx_opengl_shaders.o \
	gfx_opengl.o \
	grim.o \
	grim_controls.o \
	inputdialog.o \
	iris.o \
	keyframe.o \
	lab.o \
	lipsync.o \
	localize.o \
	lua.o \
	lua_v1.o \
	lua_v1_actor.o \
	lua_v1_graphics.o \
	lua_v1_set.o \
	lua_v1_sound.o \
	lua_v1_text.o \
	metaengine.o \
	material.o \
	model.o \
	objectstate.o \
	primitives.o \
	patchr.o \
	registry.o \
	resource.o \
	savegame.o \
	set.o \
	sector.o \
	sound.o \
	sprite.o \
	textobject.o \
	textsplit.o \
	object.o \
	debugger.o \
	md5check.o \
	md5checkdialog.o

ifdef USE_ARM_SMUSH_ASM
MODULE_OBJS += \
	movie/codecs/blocky8ARM.o
endif

# This module can be built as a plugin
ifeq ($(ENABLE_GRIM), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
