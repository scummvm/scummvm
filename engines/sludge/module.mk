MODULE := engines/sludge
 
MODULE_OBJS := \
	backdrop.o \
	bg_effects.o \
	builtin.o \
	console.o \
	cursors.o \
	debug.o \
	detection.o \
	fileset.o \
	floor.o \
	freeze.o \
	fonttext.o \
	graphics.o \
	helpers.o \
	imgloader.o \
	language.o \
	line.o \
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
	sound_openal.o \
	sprbanks.o \
	sprites.o \
	statusba.o \
	stringy.o \
	talk.o \
	thumbnail.o \
	timing.o \
	transition.o \
	variable.o \
	zbuffer.o \
	CommonCode/utf8.o \
#	linuxstuff.o \
	shaders.o \
	libwebm/mkvparser.o \
	libwebm/mkvreader.o \
 
MODULE_DIRS += \
	engines/sludge
 
# This module can be built as a plugin
ifeq ($(ENABLE_SLUDGE), DYNAMIC_PLUGIN)
PLUGIN := 1
endif
 
# Include common rules 
include $(srcdir)/rules.mk