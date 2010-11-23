# $URL$
# $Id$

MODULE := tools/create_project

MODULE_OBJS := \
	create_project.o \
	codeblocks.o \
	msvc.o \
	visualstudio.o \
	msbuild.o

# Set the name of the executable
TOOL_EXECUTABLE := create_project

# Include common rules
include $(srcdir)/rules.mk

# Silence variadic macros warning for C++ (disabled as this is included globally)
#CXXFLAGS := $(CXXFLAGS) -Wno-variadic-macros
