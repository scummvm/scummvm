
MODULE := devtools/create_nancy

MODULE_OBJS := \
	create_nancy.o \
	file.o

# Set the name of the executable
TOOL_EXECUTABLE := create_nancy

# Include common rules
include $(srcdir)/rules.mk
