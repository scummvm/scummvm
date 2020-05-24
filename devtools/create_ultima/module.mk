MODULE := devtools/create_ultima

MODULE_OBJS := \
	create_ultima.o \
	file.o \
	ultima1_resources.o \
	ultima4_resources.o

# Set the name of the executable
TOOL_EXECUTABLE := create_ultima

# Include common rules
include $(srcdir)/rules.mk
