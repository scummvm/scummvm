MODULE := sword1

MODULE_OBJS := \
	sword1/debug.o \
	sword1/eventman.o \
	sword1/logic.o \
	sword1/memman.o \
	sword1/menu.o \
	sword1/mouse.o \
	sword1/music.o \
	sword1/objectman.o \
	sword1/resman.o \
	sword1/router.o \
	sword1/screen.o \
	sword1/sound.o \
	sword1/staticres.o \
	sword1/sword1.o \
	sword1/text.o

MODULE_DIRS += \
	sword1 

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include common.rules
