MODULE := engines/titanic

MODULE_OBJS := \
	compressed_file.o \
	compression.o \
	detection.o \
	direct_draw.o \
	font.o \
	game_manager.o \
	game_view.o \
	image.o \
	main_game_window.o \
	screen_manager.o \
	simple_file.o \
	string.o \
	titanic.o \
	video_surface.o \
	core/auto_sound_event.o \
	core/dont_save_file_item.o \
	core/file_item.o \
	core/game_object.o \
	core/link_item.o \
	core/list.o \
	core/message_target.o \
	core/movie_clip.o \
	core/named_item.o \
	core/node_item.o \
	core/pet_control.o \
	core/project_item.o \
	core/resource_key.o \
	core/saveable_object.o \
	core/tree_item.o \
	core/view_item.o \
	game/announce.o \
	game/door_auto_sound_event.o \
	game/pet_position.o \
	game/room_item.o \
	game/service_elevator_door.o \
	game/sub_glass.o \
	npcs/barbot.o \
	npcs/bellbot.o \
	npcs/character.o \
	npcs/deskbot.o \
	npcs/doorbot.o \
	npcs/liftbot.o \
	npcs/maitre_d.o \
	npcs/mobile.o \
	npcs/parrot.o \
	npcs/starlings.o \
	npcs/succubus.o \
	npcs/titania.o \
	npcs/true_talk_npc.o

# This module can be built as a plugin
ifeq ($(ENABLE_TITANIC), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk
