MODULE := video

MODULE_OBJS := \
	mpeg_player.o \
	video_decoder.o
ifdef USE_BINK
MODULE_OBJS += \
	bink_decoder.o \
	bink_decoder_seek.o
endif

# Include common rules
include $(srcdir)/rules.mk
