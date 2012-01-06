MODULE := engines/myst3

MODULE_OBJS := \
	archive.o \
	console.o \
	cursor.o \
	database.o \
	detection.o \
	directoryentry.o \
	directorysubentry.o \
	hotspot.o \
	inventory.o \
	movie.o \
	myst3.o \
	node.o \
	nodecube.o \
	nodeframe.o \
	scene.o \
	script.o \
	variables.o

# This module can be built as a plugin
ifeq ($(ENABLE_MYST3), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
