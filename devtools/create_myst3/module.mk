MODULE := devtools/create_myst3

MODULE_OBJS := \
	create_myst3.o

# Set the name of the executable
TOOL_EXECUTABLE := create_myst3

# Include common rules
include $(srcdir)/rules.mk
