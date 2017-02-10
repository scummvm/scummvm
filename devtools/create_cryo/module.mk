
MODULE := devtools/create_cryo

MODULE_OBJS := \
	create_led_dat.o

# Set the name of the executable
TOOL_EXECUTABLE := create_led_dat

# Include common rules
include $(srcdir)/rules.mk
