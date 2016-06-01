
MODULE := devtools/create_titanic

MODULE_OBJS := \
	create_titanic_dat.o \
	hashmap.o \
	script_responses.o \
	str.o \
	winexe.o \
	winexe_pe.o

# Set the name of the executable
TOOL_EXECUTABLE := create_titanic

# Include common rules
include $(srcdir)/rules.mk
