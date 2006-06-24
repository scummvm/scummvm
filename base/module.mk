MODULE := base

MODULE_OBJS := \
	engine.o \
	main.o \
	commandLine.o \
	plugins.o \
	version.o

# Include common rules 
include $(srcdir)/rules.mk
