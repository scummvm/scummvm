MODULE := engines/petka

MODULE_OBJS = \
    detection.o \
    file_mgr.o \
    obj.o \
    petka.o \
    q_interface.o \
    q_manager.o \
    q_system.o \
    sound.o \
    video.o

# This module can be built as a plugin
ifeq ($(ENABLE_PETKA), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
