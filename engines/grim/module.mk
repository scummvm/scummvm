MODULE := engines/grim

MODULE_OBJS := \
	costume/bitmap_component.o \
	costume/chore.o \
	costume/colormap_component.o \
	costume/component.o \
	costume/emimesh_component.o \
	costume/emiskel_component.o \
	costume/head.o \
	costume/keyframe_component.o \
	costume/lua_var_component.o \
	costume/main_model_component.o \
	costume/material_component.o \
	costume/mesh_component.o \
	costume/model_component.o \
	costume/sound_component.o \
	costume/sprite_component.o \
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
	movie/codecs/blocky8.o \
	movie/codecs/blocky16.o \
	movie/codecs/vima.o \
	movie/codecs/smush_decoder.o \
	movie/bink.o \
	movie/mpeg.o \
	movie/smush.o \
	movie/movie.o \
	emisound/vimatrack.o \
	emisound/track.o \
	emisound/emisound.o \
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
	gfx_opengl.o \
	gfx_tinygl.o \
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
	lua_v1_sound.o \
	lua_v1_text.o \
	lua_v2.o \
	material.o \
	model.o \
	modelemi.o \
	objectstate.o \
	primitives.o \
	patchr.o \
	registry.o \
	resource.o \
	savegame.o \
	set.o \
	scx.o \
	sector.o \
	skeleton.o \
	sound.o \
	textobject.o \
	textsplit.o \
	object.o

# This module can be built as a plugin
ifeq ($(ENABLE_GRIM), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
