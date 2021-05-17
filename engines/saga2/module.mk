MODULE := engines/saga2

MODULE_OBJS := \
	actor.o \
	arrowptr.o \
	assign.o \
	audio.o \
	audiobuf.o \
	audiores.o \
	audiotmr.o \
	automap.o \
	band.o \
	beegee.o \
	blitters.o \
	button.o \
	calender.o \
	config.o \
	contain.o \
	detection.o \
	display.o \
	dispnode.o \
	dlist.o \
	document.o \
	effects.o \
	enchant.o \
	errbase.o \
	errclass.o \
	errlist.o \
	errors.o \
	exit.o \
	floating.o \
	gamemode.o \
	gdraw.o \
	gpointer.o \
	grabinfo.o \
	grequest.o \
	gtext.o \
	gtextbox.o \
	hresmgr.o \
	iff.o \
	imagcach.o \
	images.o \
	input.o \
	interp.o \
	intrface.o \
	itevideo.o \
	keybored.o \
	loadmsg.o \
	loadsave.o \
	magic.o \
	main.o \
	mainmap.o \
	mapfeatr.o \
	maprange.o \
	messager.o \
	metaengine.o \
	mission.o \
	modal.o \
	mono.o \
	motion.o \
	noise.o \
	mouseimg.o \
	nice_err.o \
	objects.o \
	objproto.o \
	osexcept.o \
	panel.o \
	path.o \
	patrol.o \
	player.o \
	playmode.o \
	pool.o \
	property.o \
	pt2angle.o \
	rect.o \
	rmem.o \
	rserver.o \
	saga2.o \
	sagafunc.o \
	savefile.o \
	sensor.o \
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
	timer.o \
	timers.o \
	tower.o \
	towerfta.o \
	transit.o \
	tromode.o \
	uidialog.o \
	vbacksav.o \
	verify.o \
	video.o \
	videobox.o \
	videomem.o \
	vidhook.o \
	vidsmk.o \
	vpal.o \
	vwdraw.o \
	vwpage.o \
	weapons.o \
	winini.o

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
