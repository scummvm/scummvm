MODULE := engines

MODULE_OBJS := \
	achievements.o \
	advancedDetector.o \
	dialogs.o \
	engine.o \
	game.o \
	metaengine.o \
	obsolete.o \
	savestate.o

# Include common rules
include $(srcdir)/rules.mk
