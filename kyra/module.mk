MODULE := kyra

MODULE_OBJS = \
	kyra/codecs.o \
	kyra/cpsimage.o \
	kyra/font.o \
	kyra/kyra.o \
	kyra/palette.o \
	kyra/resource.o \
	kyra/script_v1.o \
	kyra/script.o \
	kyra/wsamovie.o


MODULE_DIRS += \
	kyra

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
