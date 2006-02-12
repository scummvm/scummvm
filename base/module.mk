MODULE := base

MODULE_OBJS := \
	engine.o \
	gameDetector.o \
	main.o \
	plugins.o

MODULE_DIRS += \
	base

# Include common rules 
include $(srcdir)/common.rules
