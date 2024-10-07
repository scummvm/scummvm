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

#include "m4/riddle/rooms/section9/room996.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room996::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room996::init() {
	digi_preload("950_s56", -1);
	interface_hide();
	series_show("996mark", 0, 16, -1, -1, 0, 100, 0, 0);
	_flag = 1;
	if (_G(flags)[V290]) {
		_roomStates_loop0 = series_show("996 Charcoal Page", 256, 16, -1, -1, 0, 100, 0, 0);
		digi_preload("205r13a", -1);
		digi_play("205r13a", 1, 255, -1, 205);
		hotspot_set_active(_G(currentSceneDef).hotspots, "FORWARD", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "BACK", false);
	} else {
		if (player_been_here(205)) {
			_roomStates_loop0 = series_show("996 RIPPED OUT PAGE", 256, 16, -1, -1, 0, 100, 0, 0);
			hotspot_set_active(_G(currentSceneDef).hotspots, "FORWARD", false);
			hotspot_set_active(_G(currentSceneDef).hotspots, "BACK", false);
		} else {
			if (_G(flags)[V280]) {
				_roomStates_tt = series_show("996 Peru Pictograph Snake", 256, 16, -1, -1, 0, 100, 0, 0);

				if (_G(flags)[V148]) {
					_roomStates_field4 = series_show("996 Peru Pictograph Condor", 256, 16, -1, -1, 0, 100, 0, 0);
				} else {
					_roomStates_field4 = series_show("996 Peru Pictograph Spider", 256, 16, -1, -1, 0, 100, -13, -87);
				}

				if (_G(flags)[V156]) {
					_roomStates_pu = series_show("996 Peru Pictos Solved Spider", 256, 16, -1, -1, 0, 100, 0, 0);
				} else {
					_roomStates_field8 = series_show("996 Peru Pictograph Spider", 256, 16, -1, -1, 0, 100, 0, 0);
				}

				if (_G(flags)[V283]) {
					_roomStates_untie = series_show("996 Peru Pictograph Monkey", 256, 16, -1, -1, 0, 100, 0, 0);
				} else {
					_roomStates_untie = series_show("996 Peru Pictograph Spider", 256, 16, -1, -1, 0, 100, -21, 71);
				}
			}

			hotspot_set_active(_G(currentSceneDef).hotspots, "BACK", false);
		}
	}

	for (int i = 1; i < 12; ++i)
		_flagArray[i] = 0;

	if (_G(flags)[kEasterIslandCartoon])
		_flagArray[1] = 1;

	if (_G(flags)[kChinshiCartoon])
		_flagArray[1] = 2;

	if (_G(flags)[kTabletsCartoon])
		_flagArray[1] = 3;

	if (_G(flags)[kEpitaphCartoon])
		_flagArray[1] = 5;

	if (_G(flags)[kGraveyardCartoon])
		_flagArray[1] = 4;

	if (_G(flags)[kCastleCartoon])
		_flagArray[1] = 6;

	if (_G(flags)[kMocaMocheCartoon])
		_flagArray[1] = 7;

	if (_G(flags)[kTempleCartoon])
		_flagArray[1] = 8;

	if (_G(flags)[kEmeraldCartoon])
		_flagArray[1] = 9;

	_flagArray[12] = 1;
	_moveValue = 0;
}

void Room996::pre_parser() {
	if (player_said("forward", nullptr, nullptr) && (_flagArray[12] != 3)) {
		digi_play("950_s56", 1, 255, -1, 950);
		++_flagArray[12];
	}

	if (player_said("back", nullptr, nullptr) && (_flagArray[12] != 3)) {
		digi_play("950_s56", 1, 255, -1, 950);
		--_flagArray[12];
	}

	if (player_said("forward", nullptr, nullptr) && (_flagArray[12] == 3) && (_flagArray[1] == 0)) {
		++_flagArray[12];
	}

	if (player_said("forward", nullptr, nullptr) && (_flagArray[12] == 4) && (_G(flags)[V047] == 0)) {
		++_flagArray[12];
	}

	if (player_said("back", nullptr, nullptr) && (_flagArray[12] == 4) && (_G(flags)[V047] == 0)) {
		--_flagArray[12];
	}

	if (player_said("back", nullptr, nullptr) && (_flagArray[12] == 3) && (_flagArray[1] == 0)) {
		--_flagArray[12];
	}

	_flagArray[12] = CLIP(_flagArray[12], (int32)1, (int32)5);

	_G(player).need_to_walk = false;
	_G(player).ready_to_walk = true;
	_G(player).waiting_for_walk = false;
}

void Room996::parser() {
	if (player_said("--", nullptr, nullptr)) {
		_G(player).command_ready = false;
		return;
	}

	if (player_said("exit", nullptr, nullptr) && (_G(kernel).trigger >= -1)) {
		if (_G(kernel).trigger == -1) {
			player_set_commands_allowed(false);
			disable_player_commands_and_fade_init(1);
			_G(player).command_ready = false;
			return;
		}

		if (_G(kernel).trigger == 1) {
			interface_show();
			restoreAutosave();
			_G(player).command_ready = false;
			return;
		}
	}


	warning("STUB - Room996::parser not implemented");


	if (_flagArray[12] == 2) {
		if (_G(kernel).trigger >= 1) {
			
		}

		_G(player).command_ready = false;
		return;
	}




	_G(player).command_ready = false;
}

void Room996::daemon() {
	if (_G(kernel).trigger == 1)
		player_set_commands_allowed(true);
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
