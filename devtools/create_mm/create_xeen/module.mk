MODULE := devtools/create_mm

MODULE_OBJS := \
	create_xeen.o \
	cc.o \
	clouds.o \
	constants.o \
	hashmap.o \
	map.o \
	memorypool.o \
	str.o \
	swords.o

# Set the name of the executable
TOOL_EXECUTABLE := create_mm

# Include common rules
include $(srcdir)/rules.mk
