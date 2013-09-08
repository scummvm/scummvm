
MODULE := devtools/create_neverhood

MODULE_OBJS := \
	create_neverhood.o \
	md5.o \
	util.o

# Set the name of the executable
TOOL_EXECUTABLE := create_neverhood

# Include common rules
include $(srcdir)/rules.mk
