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
 * along with this program.  If not, see <http://www.gnu.org/licenses/ },.
 *
 */

#include "m4/riddle/rooms/section3/room352.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"
#include "m4/gui/gui_sys.h"
#include "m4/platform/keys.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room352::preload() {
	_G(player).walker_type = WALKER_PLAYER;
	_G(player).shadow_type = SHADOW_PLAYER;
	LoadWSAssets("OTHER SCRIPT");
}

void Room352::init() {
	_G(player).disable_hyperwalk = true;
	AddSystemHotkey(KEY_ESCAPE, escapeKeyPressed);
	_G(kernel).call_daemon_every_loop = true;

	ws_demand_location(584, 334, 3);
	ws_hide_walker();
	player_set_commands_allowed(false);
	_val1 = 0;
	_val2 = 25;

	static const char *NAMES[9] = {
		"950_s01", "352_s02", "352_s02a", "352r01",
		"352_s03", "352_s01", "352_s04", "352_s05",
		"keys"
	};
	for (int i = 0; i < 9; ++i)
		digi_preload(NAMES[i]);

	_cabStop = series_load("cab stop");
	kernel_timing_trigger(70, 10);
	kernel_timing_trigger(1, 5);
}

void Room352::daemon() {
	switch (_G(kernel).trigger) {
	case 5:
		digi_play("950_s01", 3);
		kernel_timing_trigger(900, 555);
		break;

	case 10:
		series_stream("cab pulls to a stop", 5, 0, 12);
		digi_play("352_s03", 1);
		break;

	case 12:
		kernel_timing_trigger(100, 13);
		digi_play("352_s04", 1);
		_cabStopMach = series_show_sprite("cab stop", 0, 0);
		break;

	case 13:
		digi_play("352_s01", 2, 255, 14);
		break;

	case 14:
		kernel_timing_trigger(40, 15);
		break;

	case 15:
		terminateMachineAndNull(_cabStopMach);
		series_unload(_cabStop);
		_machine1 = series_stream("cab pulls away", 5, 0, 16);
		series_stream_break_on_frame(_machine1, 32, 27);
		break;

	case 16:
		_meiChecksShoe = series_stream("mei checks her shoe", 7, 0, -1);
		series_stream_break_on_frame(_meiChecksShoe, 47, 17);
		ws_unhide_walker();
		ws_walk(319, 301, nullptr, 20, 1);
		break;

	case 17:
		series_set_frame_rate(_meiChecksShoe, 3000);
		break;

	case 20:
		player_update_info();
		_playerX = _G(player_info).x;
		_playerY = _G(player_info).y;
		_playerScale = _G(player_info).scale;

		series_ranged_play_xy("rip suit reaches for door", 1, 0, 0, 4,
			_playerX, _playerY, _playerScale, 0, 6, 21);
		_ripsh1 = series_ranged_play_xy("ripsh1", -1, 0, 0, 0,
			_playerX, _playerY, _playerScale, 0, 3000, -1);
		sendWSMessage_60000(_G(my_walker));
		digi_play("352_s02", 1);
		break;

	case 21:
		_ripDoor = series_ranged_play_xy("rip suit reaches for door", -1, 0, 4, 4,
			_playerX, _playerY, _playerScale, 0, 3000, -1);

		digi_unload("352_s05");
		digi_unload("keys");
		series_unload(0);
		series_unload(1);
		series_unload(2);
		series_unload(3);
		series_unload(4);
		series_unload(11);
		series_unload(12);
		series_unload(13);
		series_unload(14);

		_machine1 = series_stream("key to door pop-up", 9, 0, 22);
		series_stream_break_on_frame(_machine1, 14, 746);
		break;

	case 22:
		terminateMachineAndNull(_ripDoor);
		series_ranged_play_xy("rip suit reaches for door", 1, 2, 0, 4,
			_G(player_info).x, _G(player_info).y, _G(player_info).scale,
			0, 6, 24);
		break;

	case 23:
		_ripDoor = series_ranged_play_xy("rip turns from door talks",
			-1, 0, 6, 6, _playerX, _playerY, _playerScale,
			0, 3000, -1);
		digi_unload("352r01");
		digi_unload("352_s03");
		digi_unload("352_s01");
		digi_unload("352_s04");

		digi_play("950_s01", 3);
		digi_stop(1);
		kernel_timing_trigger(180, 123);
		break;

	case 24:
		series_ranged_play_xy("rip turns from door talks", 1, 0, 0, 6,
			_playerX, _playerY, _playerScale, 0, 5, 113);
		digi_play("352r01", 1, 255, 111);
		break;

	case 27:
		digi_play("352_s05", 1, 255, 555);
		series_stream_break_on_frame(_machine1, 121, 345);
		break;

	case 31:
		adv_kill_digi_between_rooms(false);
		_G(game).setRoom(303);
		break;

	case 102:
		kernel_timing_trigger(100, 30);
		break;

	case 111:
		kernel_timing_trigger(30, 112);
		break;

	case 112:
		digi_play("suspense", 2, 125);
		kernel_timing_trigger(60, 23);
		break;

	case 113:
		series_ranged_play_xy("rip turns from door talks", -1, 0, 6, 6,
			_playerX, _playerY, _playerScale, 0, 3000, -1);
		break;

	case 123:
		disable_player_commands_and_fade_init(31);
		break;

	case 345:
		digi_play("keys", 1, 100);
		break;

	case 555:
		digi_play("950_s01", 3);
		kernel_timing_trigger(900, 555);
		break;

	case 746:
		series_stream_break_on_frame(_machine1, 21, 747);
		digi_play("352_s02a", 1);
		break;

	default:
		break;
	}

	switch (_G(kernel).trigger) {
	case 55:
		series_unload(0);
		series_unload(1);
		series_unload(2);
		series_unload(3);
		series_unload(4);
		series_unload(10);
		series_unload(11);
		series_unload(12);
		series_unload(13);
		series_unload(14);
		_G(game).setRoom(304);
		break;

	case 56:
		_G(game).setRoom(494);
		break;

	default:
		break;
	}

	if (_G(MouseState).ButtonState) {
		_buttonFlag = true;
	} else if (_buttonFlag) {
		_buttonFlag = false;

		disable_player_commands_and_fade_init(55);
	}
}

void Room352::escapeKeyPressed(void *, void *) {
	_G(kernel).trigger_mode = KT_DAEMON;
	disable_player_commands_and_fade_init(56);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
