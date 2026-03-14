MODULE := engines/harvester

MODULE_OBJS = \
	harvester.o \
	console.o \
	fst_player.o \
	metaengine.o \
	resources.o \
	startup_art.o \
	startup_script.o \
	startup_text.o \
	xfile_archive.o

# This module can be built as a plugin
ifeq ($(ENABLE_HARVESTER), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
