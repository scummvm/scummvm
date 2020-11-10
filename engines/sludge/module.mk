MODULE := engines/sludge
 
MODULE_OBJS := \
	backdrop.o \
	bg_effects.o \
	builtin.o \
	console.o \
	cursors.o \
	event.o \
	fileset.o \
	floor.o \
	freeze.o \
	fonttext.o \
	function.o \
	graphics.o \
	hsi.o \
	imgloader.o \
	language.o \
	loadsave.o \
	main_loop.o \
	metaengine.o \
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
	speech.o \
	sprbanks.o \
	sprites.o \
	statusba.o \
	thumbnail.o \
	timing.o \
	transition.o \
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

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
