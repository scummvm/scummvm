MODULE := engines/lastexpress

MODULE_OBJS := \
	characters/demo/demo_abbot.o \
	characters/demo/demo_anna.o \
	characters/demo/demo_august.o \
	characters/demo/demo_cond2.o \
	characters/demo/demo_francois.o \
	characters/demo/demo_ivo.o \
	characters/demo/demo_madame.o \
	characters/demo/demo_master.o \
	characters/demo/demo_monsieur.o \
	characters/demo/demo_rebecca.o \
	characters/demo/demo_tablea.o \
	characters/demo/demo_tableb.o \
	characters/demo/demo_tablec.o \
	characters/demo/demo_tabled.o \
	characters/demo/demo_tablee.o \
	characters/demo/demo_tablef.o \
	characters/demo/demo_tatiana.o \
	characters/demo/demo_vesna.o \
	characters/demo/demo_waiter1.o \
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
	data/archive.o \
	data/cvcrfile.o \
	data/gold_archive.o \
	data/sprites.o \
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
	game/events.o \
	game/logic.o \
	game/navigation.o \
	game/nis.o \
	game/otis.o \
	game/savegame.o \
	game/vcr.o \
	menu/clock.o \
	menu/menu.o \
	sound/driver.o \
	sound/mixer.o \
	sound/slot.o \
	sound/sound.o \
	sound/subtitle.o \
	debug.o \
	graphics.o \
	lastexpress.o \
	memory.o \
	metaengine.o

# This module can be built as a plugin
ifeq ($(ENABLE_LASTEXPRESS), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
