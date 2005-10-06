MODULE := kyra

MODULE_OBJS := \
	kyra/kyra.o \
	kyra/resource.o \
	kyra/screen.o \
	kyra/script_v1.o \
	kyra/script.o \
	kyra/seqplayer.o \
	kyra/sound.o \
	kyra/staticres.o \
	kyra/sprites.o \
	kyra/wsamovie.o

MODULE_DIRS += \
	kyra

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
