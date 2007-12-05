MODULE := backends/platform/iphone

MODULE_OBJS := \
	osys_iphone.o \
	iphone_main.o \
	iphone_video.o \
	iphone_keyboard.o \
	blit_arm.o

MODULE_DIRS += \
	backends/platform/iphone/
	
# We don't use the rules.mk here on purpose
OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS)) $(OBJS)
