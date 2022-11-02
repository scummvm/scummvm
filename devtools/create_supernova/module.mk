MODULE := devtools/create_supernova

MODULE_OBJS := \
        file.o \
        po_parser.o \
        create_supernova.o

# Set the name of the executable
TOOL_EXECUTABLE := create_supernova

# Include common rules
include $(srcdir)/rules.mk

include $(srcdir)/devtools/create_supernova/create_image/module.mk
.PHONY: $(srcdir)/devtools/create_supernova/create_image/module.mk
