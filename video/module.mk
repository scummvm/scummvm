MODULE := video

MODULE_OBJS := \
	video_decoder.o \
	mpegps_decoder.o
ifdef USE_BINK
MODULE_OBJS += \
	bink_decoder.o 
endif

# Include common rules
include $(srcdir)/rules.mk
