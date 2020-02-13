
MODULE := devtools/create_cryomni3d

MODULE_OBJS := \
	create_cryomni3d_dat.o \
	util.o \
	versailles.o

# Set the name of the executable
TOOL_EXECUTABLE := create_cryomni3d_dat

# Include common rules
include $(srcdir)/rules.mk

