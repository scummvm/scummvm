MODULE := engines/pegasus

MODULE_OBJS = \
	console.o \
	credits.o \
	detection.o \
	graphics.o \
	menu.o \
	overview.o \
	pegasus.o \
	video.o \
	Game_Shell/CItem.o \
	MMShell/Sounds/MMSound.o \
	MMShell/Utilities/MMResourceFile.o \
	MMShell/Utilities/MMUtilities.o


# This module can be built as a plugin
ifeq ($(ENABLE_PEGASUS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
