MODULE := engines

MODULE_OBJS := \
	advancedDetector.o \
	dialogs.o \
	engine.o \
	game.o \
	metaengine.o \
	obsolete.o \
	savestate.o

PLUGIN := 1

# Include common rules
include $(srcdir)/rules.mk
