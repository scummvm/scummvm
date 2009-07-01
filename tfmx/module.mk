MODULE := tfmx

MODULE_OBJS := \
	mxtxplayer.o \
	tfmxplayer.o \
	tfmxdebug.o

# Include common rules
include $(srcdir)/rules.mk
