
MODULE := devtools/create_access

MODULE_OBJS := \
	create_access_dat.o \
	amazon_resources.o \
	martian_resources.o

# Set the name of the executable
TOOL_EXECUTABLE := create_access

# Include common rules
include $(srcdir)/rules.mk
