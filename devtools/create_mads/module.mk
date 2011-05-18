
MODULE := devtools/create_mads

MODULE_OBJS := \
	main.o \
	parser.o

# Set the name of the executable
TOOL_EXECUTABLE := create_mads

# Include common rules
include $(srcdir)/rules.mk
