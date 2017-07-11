MODULE := engines/sludge
 
MODULE_OBJS := \
	backdrop.o \
	bg_effects.o \
	builtin.o \
	console.o \
	cursors.o \
	detection.o \
	fileset.o \
	floor.o \
	freeze.o \
	fonttext.o \
	graphics.o \
	helpers.o \
	hsi.o \
	imgloader.o \
	language.o \
	loadsave.o \
	main_loop.o \
	memwatch.o \
	moreio.o \
	movie.o \
	newfatal.o \
	objtypes.o \
	people.o \
	region.o \
	savedata.o \
	sludge.o \
	sludger.o \
	sound.o \
	sprbanks.o \
	sprites.o \
	statusba.o \
	stringy.o \
	talk.o \
	thumbnail.o \
	timing.o \
	transition.o \
	utf8.o \
	variable.o \
	zbuffer.o \
 
MODULE_DIRS += \
	engines/sludge
 
# This module can be built as a plugin
ifeq ($(ENABLE_SLUDGE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif
 
# Include common rules 
include $(srcdir)/rules.mk