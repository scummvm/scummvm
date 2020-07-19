MODULE := engines/hadesch

MODULE_OBJS = \
    metaengine.o \
    pod_file.o \
    tag_file.o \
    pod_image.o \
    video.o \
    hadesch.o \
    baptr.o \
    rooms/olympus.o \
    rooms/walloffame.o \
    rooms/argo.o \
    rooms/crete.o \
    rooms/minos.o \
    rooms/daedalus.o \
    rooms/seriphos.o \
    rooms/medisle.o \
    rooms/troy.o \
    rooms/quiz.o \
    rooms/minotaur.o \
    rooms/catacombs.o \
    rooms/priam.o \
    rooms/athena.o \
    rooms/volcano.o \
    rooms/riverstyx.o \
    rooms/hadesthrone.o \
    rooms/credits.o \
    rooms/intro.o \
    rooms/ferry.o \
    rooms/options.o \
    rooms/monster.o \
    rooms/monster/projectile.o \
    rooms/monster/typhoon.o \
    rooms/monster/cyclops.o \
    rooms/monster/illusion.o \
    rooms/medusa.o \
    rooms/trojan.o \
    gfx_context.o \
    ambient.o \
    herobelt.o \
    hotzone.o \
    table.o \
    persistent.o

DETECT_OBJS += $(MODULE)/detection.o

# This module can be built as a plugin
ifeq ($(ENABLE_HADESCH), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
