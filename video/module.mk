MODULE := video

MODULE_OBJS := \
	mpeg_player.o \
	video_decoder.o

# Include common rules
include $(srcdir)/rules.mk
