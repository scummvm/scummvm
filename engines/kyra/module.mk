MODULE := engines/kyra

MODULE_OBJS := \
	engines/kyra/kyra.o \
	engines/kyra/resource.o \
	engines/kyra/screen.o \
	engines/kyra/script_v1.o \
	engines/kyra/script.o \
	engines/kyra/seqplayer.o \
	engines/kyra/sound.o \
	engines/kyra/staticres.o \
	engines/kyra/sprites.o \
	engines/kyra/wsamovie.o \
	engines/kyra/debugger.o \
	engines/kyra/animator.o \
	engines/kyra/gui.o \
	engines/kyra/sequences_v1.o \
	engines/kyra/items.o \
	engines/kyra/scene.o \
	engines/kyra/text.o \
	engines/kyra/timer.o \
	engines/kyra/saveload.o

MODULE_DIRS += \
	engines/kyra

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
