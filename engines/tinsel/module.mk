MODULE := engines/tinsel

MODULE_OBJS := \
    noir/lzss.o \
	actors.o \
	adpcm.o \
	anim.o \
	background.o \
	bg.o \
	bmv.o \
	cliprect.o \
	config.o \
	cursor.o \
	debugger.o \
	dialogs.o \
	drives.o \
	effect.o \
	events.o \
	faders.o \
	film.o \
	font.o \
	graphics.o \
	handle.o \
	heapmem.o \
	inv_objects.o \
	mareels.o \
	metaengine.o \
	move.o \
	movers.o \
	multiobj.o \
	music.o \
	object.o \
	palette.o \
	pcode.o \
	pdisplay.o \
	play.o \
	polygons.o \
	saveload.o \
	savescn.o \
	scene.o \
	sched.o \
	scn.o \
	scroll.o \
	sound.o \
	strres.o \
	sysvar.o \
	text.o \
	timers.o \
	tinlib.o \
	tinsel.o \
	token.o \
	noir/notebook.o \
	noir/notebook_page.o \
	noir/sysreel.o \
	noir/spriter.o \

# This module can be built as a plugin
ifeq ($(ENABLE_TINSEL), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
