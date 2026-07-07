MODULE := engines/fool

MODULE_OBJS = \
	fool.o \
	fool_cards.o \
	fool_death.o \
	fool_finale.o \
	fool_game.o \
	fool_hermit.o \
	fool_humbug.o \
	fool_jigsaw.o \
	fool_jumble.o \
	fool_justice.o \
	fool_maze.o \
	fool_metapuzzle.o \
	fool_polyomino.o \
	fool_prologue.o \
	fool_reveal.o \
	fool_sentence.o \
	fool_straightpath.o \
	fool_sun.o \
	fool_thoth.o \
	fool_threeships.o \
	fool_wordsearch.o \
	zbasic.o \
	messages.o \
	metaengine.o

# This module can be built as a plugin
ifeq ($(ENABLE_FOOL), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
