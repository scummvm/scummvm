/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "m4/core/rooms.h"
#include "m4/vars.h"

namespace M4 {

constexpr int kScaleEditor = 1;

HotSpotRec *Section::walker_spotter(int32 x, int32 y) {
	warning("TODO: walker_spotter");
	return nullptr;
}

void Sections::global_section_constructor() {
	uint sectionNum = _G(game).new_section;
	assert(sectionNum >= 1 && sectionNum <= 9);

	_activeSection = &_sections[sectionNum - 1];
}

void Sections::section_room_constructor() {
	_activeRoom = (*_activeSection)[_G(game).new_room];
}

void Sections::m4SceneLoad() {
	_G(between_rooms) = true;
	cameraShiftAmount = 0;
	cameraShift_vert_Amount = 0;
	_G(art_base_override) = nullptr;
	_G(use_alternate_attribute_file) = true;
	shut_down_digi_tracks_between_rooms = true;
	camera_pan_step = 10;
	_G(camera_reacts_to_player) = true;

	_G(kernel).force_restart = false;
	player_set_defaults();
	player_set_commands_allowed(false);		// Also sets "Wait" cursor

	//-------------------- SECTION CONSTRUCTOR and ROOM PRELOAD ------------------
	section_room_constructor();
#ifdef TODO
	_G(kernel).supress_fadeup = false;
	HEAPCHECK
		util_exec_function(room_preload_code_pointer);
	HEAPCHECK

		// we load the walker now because the head is going to seek for kernel_load_room,
		// and then it's going to seek to the walker directory, then it's going to seek
		// all the way back to the room again during the room_init_code_pointer call.
		// somewhere>roomDir>walkerDir>roomDir
		// but if we move the call it'll potentially only go somewhere>walkerDir>roomDir
		// unless they load more walkers. oh well. It'll be faster for *some* rooms.

		// Load current player walker set if not pre-loaded
		if (player.walker_in_this_scene)
			get_walker();

	//-------------------- ROOM LOAD ------------------
	intr_cancel_sentence();

	gr_pal_clear_range(master_palette, _G(kernel).first_fade, 255);

	term_message("Calling kernel_load_room");
	_G(kernel).going = kernel_load_room(_G(kernel).minPalEntry, _G(kernel).maxPalEntry, &currentSceneDef, &screenCodeBuff, &game_bgBuff);
	if (!_G(kernel).going)
		error_show(FL, 'IMP!');	// this should never ever happen

	HEAPCHECK
		get_ipl();

	// moved above. see explanation there.
	//	if (player.walker_in_this_scene)
	//		get_walker();

		// must reset event handler because loading a room re-initalizes gameBuff
	gui_buffer_set_event_handler((Buffer *)gameDrawBuff, intr_EventHandler);

	_G(kernel).trigger_mode = KT_DAEMON;
	_G(kernel).call_daemon_every_loop = false;
	_G(kernel).fade_up_time = 30;

	if (myInterface) {
		myInterface->must_redraw_all = true;
		myInterface->draw(gameInterfaceBuff);
	}

	//-------------------- GLOBAL ROOM INIT and ROOM INIT ------------------

	player_set_commands_allowed(false);
	set_commands_allowed_since_last_checked = false;

	_G(between_rooms) = false;

	global_room_init();	// supplied by game programmer
	player.walker_trigger = -1;

	if (game.previous_room == KERNEL_RESTORING_GAME)
	{
		if (player.walker_in_this_scene) {
			// if restoring game, restore player position and facing
			player_demand_location(player_info.x, player_info.y);
			player_demand_facing(player_info.facing);
		}
		// restore camera position
		MoveScreenAbs(game_buff_ptr, player_info.camera_x, player_info.camera_y);
	}

	//enable_end_user_hot_keys();
	term_message("calling room_init_code");
	util_exec_function(room_init_code_pointer);

	if (game.previous_room == KERNEL_RESTORING_GAME) {
		interface_show();
		game.previous_room = -1;
	}

	// init for fade up screen
	if (!_G(kernel).supress_fadeup) {
		pal_fade_set_start(&master_palette[0], 0);			//set fade to black instantly (0 ticks)
		pal_fade_init(&master_palette[0], _G(kernel).first_fade, 255, 100, _G(kernel).fade_up_time, 32765);      // 30 ticks
	}

	if (!set_commands_allowed_since_last_checked)
		player_set_commands_allowed(true);

	if (player_been_here(game.room_id))
		player.been_here_before = true;
	else {
		player.been_here_before = false;
		player_enters_scene(game.room_id);
	}

	//-------------------- PLAY ROOM ------------------
#endif
}

/*------------------------------------------------------------------------*/

Room *Section::operator[](uint roomNum) {
	for (uint i = 0; i < _roomsCount; ++i) {
		if (_rooms[i]._roomNum == roomNum)
			return &_rooms[i];
	}

	error("Unknown room number - %d", roomNum);
}


} // namespace M4
