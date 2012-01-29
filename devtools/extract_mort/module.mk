
MODULE := devtools/extract_mort

MODULE_OBJS := \
	extract_mort.o \

# Set the name of the executable
TOOL_EXECUTABLE := extract_mort

# Include common rules
include $(srcdir)/rules.mk
