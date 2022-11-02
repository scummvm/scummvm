
ifdef USE_ZLIB

MODULE := devtools/create_titanic

MODULE_OBJS := \
	create_titanic_dat.o \
	hashmap.o \
	memorypool.o \
	script_preresponses.o \
	script_quotes.o \
	script_ranges.o \
	script_responses.o \
	script_states.o \
	base-str.o \
	str.o \
	tag_maps.o \
	winexe.o \
	winexe_pe.o \
	zlib.o

# Set the name of the executable
TOOL_EXECUTABLE := create_titanic

TOOL_CFLAGS := $(ZLIB_CFLAGS)
TOOL_LIBS := $(ZLIB_LIBS)

# Include common rules
include $(srcdir)/rules.mk

endif
