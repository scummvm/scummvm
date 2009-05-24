MODULE := engines

MODULE_OBJS := \
	advancedDetector.o \
	engine.o \
	game.o

# Include common rules
include $(srcdir)/rules.mk
