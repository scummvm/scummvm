MODULE := engines/alg

MODULE_OBJS := \
	alg.o \
	graphics.o \
	game.o \
	metaengine.o \
	scene.o \
	video.o \
	logic/game_bountyhunter.o \
	logic/game_crimepatrol.o \
	logic/game_drugwars.o \
	logic/game_johnnyrock.o \
	logic/game_maddog.o \
	logic/game_maddog2.o \
	logic/game_spacepirates.o

MODULE_DIRS += \
	engines/alg

# This module can be built as a plugin
ifeq ($(ENABLE_ALG), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
