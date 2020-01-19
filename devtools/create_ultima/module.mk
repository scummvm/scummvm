MODULE := devtools/create_ultima

MODULE_OBJS := \
	create_ultima.o \
	archive.o \
	ultima1_map.o \
	ultima1_resources.o \
	hashmap.o \
	memorypool.o \
	str.o

# Set the name of the executable
TOOL_EXECUTABLE := create_ultima

# Include common rules
include $(srcdir)/rules.mk
