MODULE := engines/grim/smush

MODULE_OBJS := \
	blocky8.o \
	blocky16.o \
	smush.o \
	vima.o

# Include common rules
include $(srcdir)/rules.mk
