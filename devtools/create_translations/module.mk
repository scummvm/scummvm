MODULE := devtools/create_translations

MODULE_OBJS := \
	po_parser.o \
	create_translations.o

# Set the name of the executable
TOOL_EXECUTABLE := create_translations

# Include common rules
include $(srcdir)/rules.mk
