MODULE := devtools/create_supernova

MODULE_OBJS := \
        file.o \
        create_supernova.o

# Set the name of the executable
TOOL_EXECUTABLE := create_supernova

# Include common rules
include $(srcdir)/rules.mk
