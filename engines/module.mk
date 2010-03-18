MODULE := engines

MODULE_OBJS := \
	advancedDetector.o \
	dialogs.o \
	engine.o \
	game.o \
	savestate.o

# Include common rules
include $(srcdir)/rules.mk
