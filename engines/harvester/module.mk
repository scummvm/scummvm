MODULE := engines/harvester

MODULE_OBJS = \
	cft_font.o \
	harvester.o \
	npc/chessmaster_dialogue.o \
	console.o \
	dialogue.o \
	fst_player.o \
	metaengine.o \
	npc/curator_dialogue.o \
	npc/dwayne_dialogue.o \
	npc/edna_dialogue.o \
	npc/hank_dialogue.o \
	npc/herrill_dialogue.o \
	npc/jimmy_dialogue.o \
	npc/johnson_dialogue.o \
	npc/lodge_chef_dialogue.o \
	npc/mom_dialogue.o \
	npc/mother_dialogue.o \
	npc/pta_mom_dialogue.o \
	npc/stub_dialogue.o \
	npc/wasp_woman_dialogue.o \
	resources.o \
	runtime_entity.o \
	startup_art.o \
	startup_flow.o \
	startup_inventory.o \
	startup_menu.o \
	startup_room.o \
	startup_script.o \
	startup_text.o \
	xfile_archive.o

# This module can be built as a plugin
ifeq ($(ENABLE_HARVESTER), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
