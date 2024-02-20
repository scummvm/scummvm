MODULE := engines/twp

MODULE_OBJS = \
	actorlib.o \
	actorswitcher.o \
	audio.o \
	callback.o \
	camera.o \
	console.o \
	dialog.o \
	dialogs.o \
	enginedialogtarget.o \
	font.o \
	genlib.o \
	gfx.o \
	ggpack.o \
	graph.o \
	hud.o \
	ids.o \
	lighting.o \
	lip.o \
	metaengine.o \
	motor.o \
	object.o \
	objlib.o \
	prefs.o \
	resmanager.o \
	rectf.o \
	room.o \
	roomlib.o \
	scenegraph.o \
	shaders.o \
	soundlib.o \
	savegame.o \
	spritesheet.o \
	sqgame.o \
	squtil.o \
	syslib.o \
	thread.o \
	time.o \
	tsv.o \
	twp.o \
	util.o \
	vm.o \
	walkboxnode.o \
	yack.o \
	squirrel/sqapi.o \
	squirrel/sqbaselib.o \
	squirrel/sqfuncstate.o \
	squirrel/sqdebug.o \
	squirrel/sqlexer.o \
	squirrel/sqobject.o \
	squirrel/sqcompiler.o \
	squirrel/sqstate.o \
	squirrel/sqtable.o \
	squirrel/sqmem.o \
	squirrel/sqvm.o \
	squirrel/sqclass.o \
	squirrel/sqstdio.o \
	squirrel/sqstdmath.o \
	squirrel/sqstdstring.o \
	squirrel/sqstdstream.o \
	squirrel/sqstdblob.o \
	squirrel/sqstdrex.o \
	squirrel/sqstdaux.o \
	clipper/clipper.o \

ifdef USE_IMGUI
MODULE_OBJS += \
	debugtools.o \

endif

# This module can be built as a plugin
ifeq ($(ENABLE_TWP), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
