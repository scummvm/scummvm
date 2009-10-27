# $URL$
# $Id$

MODULE := tools/create_msvc

MODULE_OBJS := \
	create_msvc.o \

# Set the name of the executable
TOOL_EXECUTABLE := create_msvc

# Include common rules
include $(srcdir)/rules.mk
