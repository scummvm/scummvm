MODULE := backends/platform/ios7

MODULE_OBJS := \
	ios7_osys_main.o \
	ios7_osys_events.o \
	ios7_osys_sound.o \
	ios7_osys_video.o \
	ios7_osys_misc.o \
	ios7_main.o \
	ios7_video.o \
	ios7_keyboard.o \
	ios7_scummvm_view_controller.o \
	ios7_app_delegate.o \
	ios7_game_controller.o \
	ios7_touch_controller.o \
	ios7_mouse_controller.o \
	ios7_gamepad_controller.o

# We don't use rules.mk but rather manually update OBJS and MODULE_DIRS.
MODULE_OBJS := $(addprefix $(MODULE)/, $(MODULE_OBJS))
OBJS := $(MODULE_OBJS) $(OBJS)
MODULE_DIRS += $(sort $(dir $(MODULE_OBJS)))
