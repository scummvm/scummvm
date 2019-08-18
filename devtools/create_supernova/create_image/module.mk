MODULE := devtools/create_supernova/create_image

MODULE_OBJS := create_image.o

# Set the name of the executable
TOOL_EXECUTABLE := create_image

# Include common rules
include $(srcdir)/rules.mk

