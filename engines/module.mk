MODULE := engines

MODULE_OBJS := \
	dialogs.o \
	engine.o \
	game.o

# Include common rules
include $(srcdir)/rules.mk
