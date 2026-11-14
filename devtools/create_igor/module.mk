
MODULE := devtools/create_igor

MODULE_OBJS := \
	create_igortbl.o

# Set the name of the executable
TOOL_EXECUTABLE := create_igortbl

# Include common rules
include $(srcdir)/rules.mk
