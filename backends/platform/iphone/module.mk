MODULE := backends/platform/iphone

MODULE_OBJS := \
	osys_main.o \
	osys_events.o \
	osys_sound.o \
	osys_video.o \
	iphone_main.o \
	iphone_video.o \
	iphone_keyboard.o \
	blit_arm.o

MODULE_DIRS += \
	backends/platform/iphone/

# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)
