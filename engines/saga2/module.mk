MODULE := engines/saga2

MODULE_OBJS := \
	actor.o \
	assign.o \
	audio.o \
	automap.o \
	band.o \
	beegee.o \
	blitters.o \
	button.o \
	calender.o \
	console.o \
	contain.o \
	display.o \
	dispnode.o \
	document.o \
	effects.o \
	enchant.o \
	floating.o \
	gamemode.o \
	gdraw.o \
	gfx.o \
	gpointer.o \
	grabinfo.o \
	grequest.o \
	gtext.o \
	gtextbox.o \
	hresmgr.o \
	imagcach.o \
	interp.o \
	intrface.o \
	loadmsg.o \
	magic.o \
	main.o \
	mainmap.o \
	mapfeatr.o \
	messager.o \
	metaengine.o \
	mission.o \
	modal.o \
	motion.o \
	mouseimg.o \
	msgbox.o \
	music.o \
	objects.o \
	objproto.o \
	panel.o \
	path.o \
	patrol.o \
	player.o \
	playmode.o \
	property.o \
	pt2angle.o \
	rect.o \
	saga2.o \
	sagafunc.o \
	saveload.o \
	sensor.o \
	shorten.o \
	speech.o \
	spelcast.o \
	speldata.o \
	speldraw.o \
	spellini.o \
	spellio.o \
	spellloc.o \
	spellsiz.o \
	spellspr.o \
	spellsta.o \
	sprite.o \
	target.o \
	task.o \
	terrain.o \
	tile.o \
	tileline.o \
	tileload.o \
	tilemode.o \
	timers.o \
	tower.o \
	towerfta.o \
	transit.o \
	tromode.o \
	uidialog.o \
	vbacksav.o \
	video.o \
	videobox.o \
	vpal.o \
	vwdraw.o \
	weapons.o

MODULE_DIRS += \
	engines/saga2

# This module can be built as a plugin
ifeq ($(ENABLE_SAGA2), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
