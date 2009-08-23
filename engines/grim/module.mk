MODULE := engines/grim

MODULE_OBJS := \
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
	smush/blocky8.o \
	smush/blocky16.o \
	smush/smush.o \
	smush/vima.o \
	actor.o \
	bitmap.o \
	costume.o \
	detection.o \
	font.o \
	gfx_opengl.o \
	gfx_tinygl.o \
	grim.o \
	keyframe.o \
	lab.o \
	lipsync.o \
	localize.o \
	lua_v1.o \
	material.o \
	model.o \
	objectstate.o \
	primitives.o \
	registry.o \
	resource.o \
	savegame.o \
	scene.o \
	textobject.o \
	textsplit.o \
	walkplane.o

# This module can be built as a plugin
ifeq ($(ENABLE_GRIM), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
