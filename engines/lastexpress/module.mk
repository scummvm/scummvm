MODULE := engines/lastexpress

MODULE_OBJS := \
	data/animation.o \
	data/archive.o \
	data/background.o \
	data/cursor.o \
	data/font.o \
	data/scene.o \
	data/sequence.o \
	data/snd.o \
	data/subtitle.o \
	entities/entity.o \
	entities/abbot.o \
	entities/alexei.o \
	entities/alouan.o \
	entities/anna.o \
	entities/august.o \
	entities/boutarel.o \
	entities/chapters.o \
	entities/cooks.o \
	entities/coudert.o \
	entities/entity39.o \
	entities/francois.o \
	entities/gendarmes.o \
	entities/hadija.o \
	entities/ivo.o \
	entities/kahina.o \
	entities/kronos.o \
	entities/mahmud.o \
	entities/max.o \
	entities/mertens.o \
	entities/milos.o \
	entities/mmeboutarel.o \
	entities/pascale.o \
	entities/rebecca.o \
	entities/salko.o \
	entities/sophie.o \
	entities/tables.o \
	entities/tatiana.o \
	entities/train.o \
	entities/vassili.o \
	entities/verges.o \
	entities/vesna.o \
	entities/waiter1.o \
	entities/waiter2.o \
	entities/yasmin.o \
	fight/fight.o \
	fight/fighter.o \
	fight/fighter_anna.o \
	fight/fighter_ivo.o \
	fight/fighter_milos.o \
	fight/fighter_salko.o \
	fight/fighter_vesna.o \
	game/action.o \
	game/beetle.o \
	game/entities.o \
	game/inventory.o \
	game/logic.o \
	game/object.o \
	game/savegame.o \
	game/savepoint.o \
	game/scenes.o \
	game/state.o \
	menu/clock.o \
	menu/menu.o \
	menu/trainline.o \
	sound/entry.o \
	sound/queue.o \
	sound/sound.o \
	debug.o \
	graphics.o \
	lastexpress.o \
	metaengine.o \
	resource.o

# This module can be built as a plugin
ifeq ($(ENABLE_LASTEXPRESS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
