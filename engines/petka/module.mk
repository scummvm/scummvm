MODULE := engines/petka

MODULE_OBJS = \
    big_dialogue.o \
    file_mgr.o \
    flc.o \
    metaengine.o \
    petka.o \
    saveload.o \
    q_manager.o \
    q_system.o \
    sound.o \
    video.o \
    walk.o \
    interfaces/dialog_interface.o \
    interfaces/interface.o \
    interfaces/main.o \
    interfaces/map.o \
    interfaces/panel.o \
    interfaces/save_load.o \
    interfaces/sequence.o \
    interfaces/startup.o \
    objects/object.o \
    objects/object_bg.o \
    objects/object_case.o \
    objects/object_cursor.o \
    objects/object_star.o \
    objects/heroes.o \
    objects/text.o

# This module can be built as a plugin
ifeq ($(ENABLE_PETKA), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
