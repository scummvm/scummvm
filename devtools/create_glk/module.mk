
MODULE := devtools/create_glk

MODULE_OBJS := \
	create_glk.o

# Set the name of the executable
TOOL_EXECUTABLE := create_glk

# Include common rules
include $(srcdir)/rules.mk
