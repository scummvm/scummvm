MODULE := devtools/create_supernova2

MODULE_OBJS := \
        file.o \
        po_parser.o \
        create_supernova2.o

# Set the name of the executable
TOOL_EXECUTABLE := create_supernova2

# Include common rules
include $(srcdir)/rules.mk
