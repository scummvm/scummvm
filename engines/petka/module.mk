MODULE := engines/petka

MODULE_OBJS = \
    detection.o \
    file_mgr.o \
    flc.o \
    petka.o \
    q_manager.o \
    q_system.o \
    sound.o \
    video.o \
    interfaces/interface.o \
    interfaces/main.o \
    interfaces/panel.o \
    interfaces/save_load.o \
    interfaces/startup.o \
    objects/object.o \
    objects/object_bg.o \
    objects/object_case.o \
    objects/object_cursor.o \
    objects/object_star.o

# This module can be built as a plugin
ifeq ($(ENABLE_PETKA), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
