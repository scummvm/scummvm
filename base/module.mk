MODULE := base

MODULE_OBJS := \
	main.o \
	commandLine.o \
	game.o \
	plugins.o \
	version.o

# Include common rules 
include $(srcdir)/rules.mk
