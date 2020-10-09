MODULE := engines/dragons

MODULE_OBJS := \
	actor.o \
	actorresource.o \
	background.o \
	bag.o \
	bigfile.o \
	credits.o \
	cursor.o \
	cutscene.o \
	dragonflg.o \
	dragonimg.o \
	dragonini.o \
	dragonobd.o \
	dragonrms.o \
	dragonvar.o \
	dragons.o \
	font.o \
	inventory.o \
	metaengine.o \
	midimusicplayer.o \
	minigame1.o \
	minigame2.o \
	minigame3.o \
	minigame4.o \
	minigame5.o \
	saveload.o \
	scene.o \
	screen.o \
	scriptopcodes.o \
	sequenceopcodes.o \
	specialopcodes.o \
	sound.o \
	talk.o \
	vabsound.o \
	strplayer.o

# This module can be built as a plugin
ifeq ($(ENABLE_DRAGONS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
