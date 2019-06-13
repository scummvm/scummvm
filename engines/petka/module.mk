MODULE := engines/petka

MODULE_OBJS = \
    detection.o \
    file_mgr.o \
    flc.o \
    obj.o \
    petka.o \
    q_manager.o \
    q_system.o \
    sound.o \
    video.o \
    interfaces/interface.o \
    interfaces/main.o \
    interfaces/save_load.o \
    interfaces/startup.o

# This module can be built as a plugin
ifeq ($(ENABLE_PETKA), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
