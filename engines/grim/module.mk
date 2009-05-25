MODULE := engines/grim

MODULE_OBJS := \
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
	lua.o \
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

# Include common rules
include $(srcdir)/rules.mk
