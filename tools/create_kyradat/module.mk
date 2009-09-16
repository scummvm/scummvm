# $URL$
# $Id$

MODULE := tools/create_kyradat

MODULE_OBJS := \
	create_kyradat.o \
	pak.o \
	md5.o \
	util.o \
	search.o

# Set the name of the executable
TOOL_EXECUTABLE := create_kyradat

# Include common rules
include $(srcdir)/rules.mk
