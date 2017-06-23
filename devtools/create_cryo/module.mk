
MODULE := devtools/create_cryo

MODULE_OBJS := \
	create_cryo_dat.o

# Set the name of the executable
TOOL_EXECUTABLE := create_cryo_dat

# Include common rules
include $(srcdir)/rules.mk
