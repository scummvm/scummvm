MODULE := engine/smush

MODULE_OBJS := \
	blocky16.o \
	smush.o \
	vima.o

# Include common rules
include $(srcdir)/rules.mk
