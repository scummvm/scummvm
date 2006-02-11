MODULE := engines/sword1

MODULE_OBJS := \
	engines/sword1/animation.o \
	engines/sword1/control.o \
	engines/sword1/credits.o \
	engines/sword1/debug.o \
	engines/sword1/eventman.o \
	engines/sword1/logic.o \
	engines/sword1/memman.o \
	engines/sword1/menu.o \
	engines/sword1/mouse.o \
	engines/sword1/music.o \
	engines/sword1/objectman.o \
	engines/sword1/resman.o \
	engines/sword1/router.o \
	engines/sword1/screen.o \
	engines/sword1/sound.o \
	engines/sword1/staticres.o \
	engines/sword1/sword1.o \
	engines/sword1/text.o

MODULE_DIRS += \
	engines/sword1 

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
