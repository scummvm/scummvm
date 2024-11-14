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
#include "m4/core/errors.h"
#include "m4/core/imath.h"
#include "m4/adv_r/adv_been.h"
#include "m4/adv_r/adv_control.h"
#include "m4/adv_r/adv_file.h"
#include "m4/adv_r/adv_interface.h"
#include "m4/adv_r/adv_scale.h"
#include "m4/adv_r/adv_walk.h"
#include "m4/adv_r/db_env.h"
#include "m4/adv_r/other.h"
#include "m4/fileio/extensions.h"
#include "m4/graphics/krn_pal.h"
#include "m4/gui/gui_buffer.h"
#include "m4/gui/gui_sys.h"
#include "m4/gui/gui_vmng.h"
#include "m4/wscript/wst_regs.h"
#include "m4/vars.h"
#include "m4/m4.h"

namespace M4 {

void Room::preload() {
	_G(player).walker_in_this_scene = true;
}

void Room::parser() {
	_G(kernel).trigger = KT_DAEMON;
}


void Sections::global_section_constructor() {
	uint sectionNum = _G(game).new_section;
	assert(sectionNum >= 1 && sectionNum <= 9);

	_activeSection = _sections[sectionNum - 1];
	assert(_activeSection);
}

void Sections::section_room_constructor() {
	_activeRoom = (*_activeSection)[_G(game).new_room];
	assert(_activeRoom);
}

void Sections::game_daemon_code() {
	_G(kernel).trigger_mode = KT_DAEMON;
	_G(kernel).continue_handling_trigger = false;

	room_daemon();

	if (_G(kernel).continue_handling_trigger) {
		_G(kernel).continue_handling_trigger = false;
		daemon();
	}

	if (_G(kernel).continue_handling_trigger)
		global_daemon();

	if (_G(kernel).trigger == TRIG_RESTORE_GAME) {
		_G(game).room_id = -1;
		_G(game).section_id = -1;
		_G(game).previous_room = KERNEL_RESTORING_GAME;
	}
}

void Sections::m4SceneLoad() {
	_G(between_rooms) = true;
	_cameraShiftAmount = 0;
	_cameraShift_vert_Amount = 0;
	_G(art_base_override) = nullptr;
	_G(use_alternate_attribute_file) = true;
	_G(shut_down_digi_tracks_between_rooms) = true;
	camera_pan_step = 10;
	_G(camera_reacts_to_player) = true;

	_G(kernel).force_restart = false;
	player_set_defaults();
	player_set_commands_allowed(false);		// Also sets "Wait" cursor

	// -------------------- SECTION CONSTRUCTOR and ROOM PRELOAD ------------------

	section_room_constructor();
	_G(kernel).suppress_fadeup = false;

	room_preload();

	// -------------------- ROOM LOAD ------------------

	_GI().cancel_sentence();
	gr_pal_clear_range(_G(master_palette), _G(kernel).first_fade, 255);

	term_message("Calling kernel_load_room");

	_G(kernel).going = kernel_load_room(MIN_PAL_ENTRY, MAX_PAL_ENTRY,
		&_G(currentSceneDef), &_G(screenCodeBuff), &_G(game_bgBuff));
	if (!_G(kernel).going)
		error_show(FL, 'IMP!');	// this should never ever happen

	get_ipl();

	// Must reset event handler because loading a room re-initalizes gameBuff
	gui_buffer_set_event_handler(_G(gameDrawBuff), intr_EventHandler);

	if (_G(player).walker_in_this_scene)
		get_walker();

	_G(kernel).trigger_mode = KT_DAEMON;
	_G(kernel).call_daemon_every_loop = false;
	_G(kernel).fade_up_time = 30;

	//-------------------- GLOBAL ROOM INIT and ROOM INIT ------------------

	player_set_commands_allowed(false);
	_G(set_commands_allowed_since_last_checked) = false;
	_G(between_rooms) = false;

	global_room_init();
	_G(player).walker_trigger = -1;

	if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
		if (_G(player).walker_in_this_scene) {
			// If restoring game, restore player position and facing
			player_demand_location(_G(player_info).x, _G(player_info).y);
			player_demand_facing(_G(player_info).facing);
		}

		// Restore camera position
		MoveScreenAbs(_G(game_buff_ptr), _G(player_info).camera_x, _G(player_info).camera_y);
	}

	_G(player).been_here_before = player_been_here(_G(game).room_id);

	term_message("calling room_init_code");
	room_init();

	if (_G(game).previous_room == KERNEL_RESTORING_GAME) {
		_G(game).previous_room = -1;
	}

	// Init for fade up screen
	if (!_G(kernel).suppress_fadeup) {
		pal_fade_set_start(&_G(master_palette)[0], 0);	// Set fade to black instantly (0 ticks)
		pal_fade_init(&_G(master_palette)[0], _G(kernel).first_fade, 255, 100,
			_G(kernel).fade_up_time, 32765);      // 30 ticks
	}

	if (!_G(set_commands_allowed_since_last_checked))
		player_set_commands_allowed(true);

	//-------------------- PRE-PLAY ROOM ------------------

	term_message("Off to the races -- %d", timer_read_60());
}

void Sections::m4RunScene() {
	if (!player_been_here(_G(game).room_id))
		player_enters_scene(_G(game).room_id);

	game_control_cycle();
}

void Sections::m4EndScene() {
	_G(between_rooms) = true;
	hotspot_unhide_and_dump();

	if (!_G(kernel).going && _GI()._visible && player_commands_allowed())
		other_save_game_for_resurrection();

	if (_G(kernel).teleported_in) {
		_G(kernel).teleported_in = false;
		pal_fade_set_start(&_G(master_palette)[0], 0);			// Set fade to black instantly (0 ticks)
	}

	//-------------------- cancel all editors ------------------

	scale_editor_cancel();

	//-------------------- ROOM SHUTDOWN CODE ------------------

	term_message("Shuttin' down the scene");
	room_shutdown();
	kernel_unload_room(&_G(currentSceneDef), &_G(screenCodeBuff), &_G(game_bgBuff));

	pal_cycle_stop();

	if (_G(shut_down_digi_tracks_between_rooms)) {
		_G(digi).stop(1);
		_G(digi).stop(2);
		_G(digi).stop(3);
		_G(digi).flush_mem();
	}

	conv_unload(conv_get_handle());

	ws_KillDeadMachines();

	//-------------------- DUMP ASSETS AND MINI-ENGINES ------------------
	// Note machines should always be cleared before anything else
	ClearWSAssets(_WS_ASSET_MACH, 0, 255);
	ClearWSAssets(_WS_ASSET_SEQU, 0, 255);
	ClearWSAssets(_WS_ASSET_DATA, 0, 255);
	ClearWSAssets(_WS_ASSET_CELS, 0, 255);

	// Dump a list of any resources remaining in memory
	_G(resources).dumpResources();

	// Reload the walker and show scripts.
	if (!LoadWSAssets("walker script", &_G(master_palette)[0]))
		error_show(FL, 'FNF!', "walker script");
	if (!LoadWSAssets("show script", &_G(master_palette)[0]))
		error_show(FL, 'FNF!', "show script");
	if (!LoadWSAssets("stream script", &_G(master_palette)[0]))
		error_show(FL, 'FNF', "stream script");

	g_vars->global_menu_system_init();
}

void Sections::get_ipl() {
	if (_G(inverse_pal))
		delete _G(inverse_pal);
	_G(inverse_pal) = nullptr;

	char *name;
	Common::String filename;

	name = env_find(_G(currentSceneDef).art_base);
	if (name) {
		// Means found in database
		filename = f_extension_new(name, "ipl");

	} else {
		// Concat hag mode
		filename = Common::String::format("%s.IPL", _G(currentSceneDef).art_base);
	}

	_G(inverse_pal) = new InvPal(filename.c_str());
	if (!_G(inverse_pal))
		error_show(FL, 'OOM!', "loading ipl: %s", filename.c_str());
}

void Sections::get_walker() {
	term_message("Loading walker sprites");
	if (!_GW().walk_load_walker_and_shadow_series())
		error_show(FL, 'WLOD');
	ws_walk_init_system();
}

void Sections::game_control_cycle() {
	int32 status;

	while (_G(game).new_room == _G(game).room_id && _G(kernel).going && !_G(kernel).force_restart) {
		krn_pal_game_task();

		ScreenContext *screen = vmng_screen_find(_G(gameDrawBuff), &status);
		if (!screen)
			error_show(FL, 'BUF!');

		if (_G(player).ready_to_walk) {
			if (_G(player).need_to_walk) {
				if (_G(player).noun[0] == '@' || !_G(player).walker_in_this_scene) {
					term_message("parsing0");
					parse_player_command_now();
					term_message("parsed0");
				} else {
					term_message("player: walk to (%d, %d), facing: %d",
						_G(player).walk_x, _G(player).walk_y, _G(player).walk_facing);

					if (_G(player).walk_x < 0 || _G(player).walk_y < 0) {
						term_message("walk x or y < 0 - player: %s %s %s",
							_G(player).verb, _G(player).noun, _G(player).prep);
					}

					_G(player).waiting_for_walk = true;
					ws_walk(_G(my_walker), _G(player).walk_x, _G(player).walk_y,
						nullptr, _G(player).walker_trigger, _G(player).walk_facing);
					term_message("walked");

					_G(player).need_to_walk = false;
				}
			} else if (!_G(player).waiting_for_walk) {
				term_message("parsing1");
				parse_player_command_now();
				term_message("parsed0");

				_G(player).ready_to_walk = false;
			}
		}

		if (_G(player).walker_in_this_scene && _G(camera_reacts_to_player) &&
				_G(gameDrawBuff)->w > 640 && _G(my_walker)) {
			int xp = (_G(my_walker)->myAnim8->myRegs[IDX_X] >> 16) + screen->x1;

			if (xp > 560 && _cameraShiftAmount >= 0) {
				_cameraShiftAmount += screen->x1 - 427;
				int xv = _cameraShiftAmount + _G(gameDrawBuff)->w - 1;

				if (xv < 639)
					_cameraShiftAmount = -(_G(gameDrawBuff)->w - 640);
				_cameraShiftAmount -= screen->x1;
			} else if (xp < 80 && _cameraShiftAmount <= 0) {
				_cameraShiftAmount += screen->x1 + 427;

				if (_cameraShiftAmount > 0)
					_cameraShiftAmount = 0;

				_cameraShiftAmount -= screen->x1;
			}
		}

		// Ensure the screen is updated
		g_system->updateScreen();
		g_system->delayMillis(10);

		if (g_engine->shouldQuit())
			_G(kernel).going = false;
	}

	_GI().cancel_sentence();
}

void Sections::parse_player_command_now() {
	if (_G(player).command_ready) {
		term_message("player: %s %s %s", _G(player).verb, _G(player).noun, _G(player).prep);
		_G(cursor_state) = kARROW;
		_G(kernel).trigger_mode = KT_PARSE;

		room_parser();

		if (_G(player).command_ready) {
			section_parser();

			if (_G(player).command_ready) {
				global_parser();

				if (_G(player).command_ready) {
					room_error();

					if (_G(player).command_ready)
						global_error_code();
				}
			}
		}

		term_message("...parsed");
	}
}

void Sections::pal_game_task() {
	int32 status;
	bool updateVideo;
	int delta = 0;
	Common::String line;

	if (!player_commands_allowed())
		mouse_set_sprite(5);

	ScreenContext *game_buff_ptr = vmng_screen_find(_G(gameDrawBuff), &status);

	if (!_G(kernel).pause) {
		if (_G(toggle_cursor) != CURSCHANGE_NONE) {
			CursorChange change = _G(toggle_cursor);
			_G(toggle_cursor) = CURSCHANGE_NONE;
			g_vars->getHotkeys()->toggle_through_cursors(change);
		}

		updateVideo = !_cameraShiftAmount && !_cameraShift_vert_Amount;

		cycleEngines(_G(game_bgBuff)->get_buffer(), &(_G(currentSceneDef).depth_table[0]),
			_G(screenCodeBuff)->get_buffer(), (uint8 *)&_G(master_palette)[0], _G(inverse_pal)->get_ptr(), updateVideo);

		_G(inverse_pal)->release();
		_G(game_bgBuff)->release();

		if (!game_buff_ptr)
			error_show(FL, 'BUF!');

		if (_cameraShiftAmount) {
			if (_G(kernel).camera_pan_instant) {
				delta = _cameraShiftAmount;
				_cameraShiftAmount = 0;
			} else {
				if (_cameraShiftAmount > 0) {
					delta = imath_min(_cameraShiftAmount, camera_pan_step);
				} else {
					delta = imath_max(_cameraShiftAmount, -camera_pan_step);
				}

				_cameraShiftAmount -= delta;
			}

			MoveScreenDelta(game_buff_ptr, delta, 0);
		}

		if (_cameraShift_vert_Amount) {
			if (_G(kernel).camera_pan_instant) {
				delta = _cameraShift_vert_Amount;
				_cameraShift_vert_Amount = 0;
			} else {
				if (_cameraShift_vert_Amount > 0) {
					delta = imath_min(_cameraShift_vert_Amount, camera_pan_step);
				} else {
					delta = imath_max(_cameraShift_vert_Amount, camera_pan_step);
				}

				_cameraShift_vert_Amount -= delta;
			}
		}
	}

	pal_fx_update();
	_G(digi).read_another_chunk();
	_G(midi).loop();

	gui_system_event_handler();

	if (conv_is_event_ready()) {
		_G(player).command_ready = true;
		term_message("conv parse row");
		parse_player_command_now();
		term_message("conv parse finish");

		(void)conv_get_event();
	}

	f_stream_Process(2);

	if (_G(kernel).call_daemon_every_loop)
		tick();

	if (_G(editors_in_use) && (_G(editors_in_use) & 1))
		scale_editor_draw();

	if (_G(showMousePos))
		update_mouse_pos_dialog();
}

void Sections::camera_shift_xy(int32 x, int32 y) {
	int32 status;
	ScreenContext *sc = vmng_screen_find(_G(gameDrawBuff), &status);
	assert(sc);

	_cameraShiftAmount = -sc->x1 - x + _G(kernel).letter_box_x;
	_cameraShift_vert_Amount = -sc->y1 - y + _G(kernel).letter_box_y;
}

void Sections::adv_camera_pan_step(int32 step) {
	camera_pan_step = step;
}

Room *Sections::getRoom(int room) const {
	return (*_sections[(room / 100) - 1])[room];
}

/*------------------------------------------------------------------------*/

Room *Section::operator[](uint roomNum) {
	Room *room = _rooms[roomNum];
	if (!room)
		error("Unknown room number - %d", roomNum);

	return room;
}

} // namespace M4
