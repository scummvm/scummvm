MODULE := engines/mutationofjb

MODULE_OBJS := \
	encryptedfile.o \
	detection.o \
	mutationofjb.o \
	room.o

# This module can be built as a plugin
ifeq ($(ENABLE_MUTATIONOFJB), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
