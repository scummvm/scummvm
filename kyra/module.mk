MODULE := kyra

MODULE_OBJS = \
	kyra/kyra.o kyra/codecs.o kyra/script.o kyra/script_v1.o kyra/resource.o \
	kyra/wsamovie.o kyra/palette.o kyra/cpsimage.o kyra/font.o

MODULE_DIRS += \
	kyra

# This module can be built as a plugin
ifdef BUILD_PLUGINS
PLUGIN := 1
endif

# Include common rules 
include $(srcdir)/common.rules
