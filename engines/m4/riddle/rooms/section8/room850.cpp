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

#include "m4/riddle/rooms/section8/room850.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room850::preload() {
	Room::preload();
	_G(player).walker_in_this_scene = false;
}

void Room850::init() {
	player_set_commands_allowed(false);

	static const char *const PRELOADS[8] = {
		"850_s01", "850_s02", "850_s03", "850_s04", "850_s05",
		"850_s06", "603_s02c", "801_s02"
	};
	for (int i = 0; i < 8; ++i)
		digi_preload(PRELOADS[i]);

	_allStuff = series_load("850 ALL STUFF");
	digi_play_loop("603_s02c", 2, 100, -1, 603);
	_allStuffMach = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0x100, false,
		triggerMachineByHashCallback, "850 all");
	sendWSMessage_10000(1, _allStuffMach, _allStuff, 1, 1, 1, _allStuff, 1, 15, 4);
}

void Room850::daemon() {
	switch (_G(kernel).trigger) {
	case 1:
		kernel_timing_trigger(110, 10);
		break;

	case 10:
		digi_play("850_s01", 1);
		kernel_timing_trigger(360, 15);
		break;

	case 15:
		digi_stop(2);
		digi_play("850_s03", 2, 255, 20);
		break;

	case 20:
		sendWSMessage_10000(1, _allStuffMach, _allStuff, 16, 20, 25, _allStuff, 20, 20, 0);
		digi_play("850_s07", 3);
		break;

	case 25:
		sendWSMessage_10000(1, _allStuffMach, _allStuff, 20, 29, 30, _allStuff, 29, 29, 0);
		digi_play("850_s05", 2);
		break;

	case 30:
		digi_play("850_s05", 2, 255, 40);
		break;

	case 40:
		digi_play("850_s02", 1);
		kernel_timing_trigger(60, 50);
		break;

	case 50:
		digi_play("850r01", 2, 255, 60);
		break;

	case 60:
		digi_play_loop("801_s02", 3, 100);
		kernel_timing_trigger(15, 100);
		break;


	case 100:
		player_set_commands_allowed(false);
		disable_player_commands_and_fade_init(110);
		break;

	case 110:
		adv_kill_digi_between_rooms(false);
		_G(game).setRoom(801);
		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
