MODULE := backends/platform/ios7

MODULE_OBJS := \
	ios7_osys_main.o \
	ios7_osys_events.o \
	ios7_osys_sound.o \
	ios7_osys_video.o \
	ios7_main.o \
	ios7_video.o \
	ios7_keyboard.o \
	iOS7ScummVMViewController.o \
	iOS7AppDelegate.o

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
