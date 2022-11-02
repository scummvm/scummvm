
MODULE := devtools/create_mortdat

MODULE_OBJS := \
	create_mortdat.o \

# Set the name of the executable
TOOL_EXECUTABLE := create_mortdat

# Include common rules
include $(srcdir)/rules.mk
