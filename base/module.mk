MODULE := base

MODULE_OBJS := \
	engine.o \
	main.o \
	options.o \
	plugins.o \
	version.o

MODULE_DIRS += \
	base

# Include common rules 
include $(srcdir)/common.rules
