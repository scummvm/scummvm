MODULE := devtools/create_engine

MODULE_OBJS := \
	create_engine.o

# Set the name of the executable
TOOL_EXECUTABLE := create_engine

# Include common rules
include $(srcdir)/rules.mk
