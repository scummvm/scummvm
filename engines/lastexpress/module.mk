MODULE := engines/lastexpress

MODULE_OBJS := \
	characters/abbot.o \
	characters/alexei.o \
	characters/alouan.o \
	characters/anna.o \
	characters/august.o \
	characters/clerk.o \
	characters/cond1.o \
	characters/cond2.o \
	characters/cook.o \
	characters/francois.o \
	characters/hadija.o \
	characters/headwait.o \
	characters/ivo.o \
	characters/kahina.o \
	characters/kronos.o \
	characters/madame.o \
	characters/mahmud.o \
	characters/master.o \
	characters/max.o \
	characters/milos.o \
	characters/mitchell.o \
	characters/monsieur.o \
	characters/police.o \
	characters/rebecca.o \
	characters/salko.o \
	characters/sophie.o \
	characters/tablea.o \
	characters/tableb.o \
	characters/tablec.o \
	characters/tabled.o \
	characters/tablee.o \
	characters/tablef.o \
	characters/tatiana.o \
	characters/trainm.o \
	characters/vassili.o \
	characters/vesna.o \
	characters/waiter1.o \
	characters/waiter2.o \
	characters/yasmin.o \
	data/animation.o \
	data/archive.o \
	data/background.o \
	data/cursor.o \
	data/cvcrfile.o \
	data/font.o \
	data/scene.o \
	data/sequence.o \
	data/snd.o \
	data/sprites.o \
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
	fight/fighter_cath.o \
	fight/opponent.o \
	fight/opponent_ivo.o \
	fight/opponent_milos.o \
	fight/opponent_salko.o \
	fight/opponent_vesna1.o \
	fight/opponent_vesna2.o \
	game/action.o \
	game/beetle.o \
	game/credits.o \
	game/entities.o \
	game/events.o \
	game/inventory.o \
	game/logic.o \
	game/navigation.o \
	game/nis.o \
	game/object.o \
	game/otis.o \
	game/savegame.o \
	game/savepoint.o \
	game/scenes.o \
	game/state.o \
	game/vcr.o \
	menu/clock.o \
	menu/menu.o \
	menu/trainline.o \
	sound/driver.o \
	sound/entry.o \
	sound/mixer.o \
	sound/queue.o \
	sound/slot.o \
	sound/sound.o \
	sound/subtitle.o \
	debug.o \
	graphics.o \
	lastexpress.o \
	memory.o \
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
