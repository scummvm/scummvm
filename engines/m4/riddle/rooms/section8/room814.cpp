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

#include "m4/riddle/rooms/section8/room814.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/riddle.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room814::init() {
	_currentRoom = 814;
	if (_G(flags)[V276] == 0) {
		ws_walk_load_walker_series(S8_SHADOW_DIRS1, S8_SHADOW_NAMES1, false);
		ws_walk_load_walker_series(S8_SHADOW_DIRS2, S8_SHADOW_NAMES2, false);
	}

	series_plain_play("814 lit urn 1", -1, 0, 100, 32767, 7, -1, false);
	series_plain_play("814 lit urn 2", -1, 0, 100, 32767, 7, -1, false);
	_var1 = 3;
	_var2 = 2;
	_var3 = 3;
	_var4 = 7;
	_var5 = -1;

	getSeriesName(2, false);
	_machArr[0] = series_play(_currentSeriesName.c_str(), 767, 16, -1, 0, 0, 100, 0, 0, 0, -1);
	getSeriesName(3, false);
	_machArr[1] = series_play(_currentSeriesName.c_str(), 767, 16, -1, 0, 0, 100, 0, 0, 0, -1);
	getSeriesName(7, false);
	_machArr[2] = series_play(_currentSeriesName.c_str(), 767, 16, -1, 0, 0, 100, 0, 0, 0, -1);
	_machArr[3] = nullptr;
	hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN", false);
	hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN ", false);
	hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN  ", false);
	hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN   ", false);
	hotspot_set_active(_G(currentSceneDef).hotspots, "MEI CHEN    ", false);

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		player_set_commands_allowed(true);
		digi_preload("950_s29", -1);
		if (_G(flags)[V276] == 0) {
			_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, _guessX, 400, _guessIndex, Walker::player_walker_callback, "mc_trek");
			kernel_timing_trigger(1, 1, nullptr);
		}
		break;

	case 803:
		player_set_commands_allowed(false);
		ws_demand_facing(_G(my_walker), 3);
		ws_demand_location(_G(my_walker), -10, 325);
		if (_G(flags)[V276] == 0) {
			_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, -60, 325, 2, Walker::player_walker_callback, "mc_trek");
			_unkArrayIndex = 0;
			_guessX = 100;
			_guessIndex = 1;
			ws_walk(_G(my_walker), 150, 355, nullptr, -1, 3, true);
			ws_walk(_mcTrekMach, 80, 400, nullptr, 1, 1, true);
		} else {
			ws_walk(_G(my_walker), 150, 355, nullptr, 1, 3, true);
		}
		break;
	default:
		player_set_commands_allowed(false);
		moveScreen(1280, 0);
		ws_demand_facing(_G(my_walker), 9);
		ws_demand_location(_G(my_walker), 1925, 325);
		if (_G(flags)[V276] == 0) {
			_mcTrekMach = triggerMachineByHash_3000(8, 4, *S8_SHADOW_DIRS2, *S8_SHADOW_DIRS1, 2000, 325, 9, Walker::player_walker_callback, "mc_trek");
			_unkArrayIndex = 4;
			_guessX = 1815;
			_guessIndex = 11;
			ws_walk(_G(my_walker), 1765, 348, nullptr, 0, 9, true);
			DisposePath(_mcTrekMach->walkPath);
			_mcTrekMach->walkPath = CreateCustomPath(1832, 325, 1815, 400, 0xffff);
			ws_custom_walk(_mcTrekMach, 11, 1, true);
		} else {
			ws_walk(_G(my_walker), 1765, 348, nullptr, 1, 9, true);
		}

		break;
	}

	digi_play_loop("950_s29", 3, 96, -1, -1);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
