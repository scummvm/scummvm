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

#include "m4/burger/rooms/section1/room101.h"
#include "m4/burger/wilbur.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"
#include "m4/core/imath.h"

namespace M4 {
namespace Burger {
namespace Rooms {

void Room101::init() {
	_G(player).walker_in_this_scene = true;
	_val1 = 255;

	digi_stop(1);
	digi_preload("101_001");
	_G(kernel).call_daemon_every_loop = true;

	if (_G(game).previous_room != 102)
		door();

	switch (_G(game).previous_room) {
	case -2:
		// Do nothing
		break;

	case 102:
		player_set_commands_allowed(false);
		if (_G(flags)[ROOM101_FLAG1]) {
			_G(flags)[ROOM101_FLAG1] = 0;
			ws_demand_facing(_G(my_walker), 10);
			ws_demand_location(_G(my_walker), 338, 265);
			_G(roomVal1) = 16;

		} else {
			ws_demand_facing(_G(my_walker), 4);
			ws_demand_location(_G(my_walker), 264, 259);
			_G(roomVal1) = 10;
		}

		ws_hide_walker(_G(my_walker));
		kernel_trigger_dispatch_now(10015);
		break;

	case 103:
		player_set_commands_allowed(false);
		ws_demand_facing(_G(my_walker), 1);
		ws_demand_location(_G(my_walker), 197, 276);
		ws_hide_walker(_G(my_walker));

		_G(roomVal1) = 6;
		kernel_trigger_dispatch_now(10016);
		break;

	case 104:
		_G(roomVal1) = 2;
		kernel_trigger_dispatch_now(10016);
		break;

	case 106:
		_G(roomVal1) = 3;
		kernel_trigger_dispatch_now(10016);
		break;

	case 135:
		_G(roomVal1) = 4;
		kernel_trigger_dispatch_now(10016);
		break;

	case 142:
		_G(roomVal1) = 5;
		kernel_trigger_dispatch_now(10016);
		break;

	default:
		ws_demand_location(_G(my_walker), 320, 271);
		ws_demand_facing(_G(my_walker), 5);

		if (_G(game).previous_section > 1) {
			if (_G(flags)[ROOM101_FLAG2]) {
				ws_demand_location(_G(my_walker), 280, 309);
				ws_demand_facing(_G(my_walker), 8);
				player_set_commands_allowed(false);
			}

			kernel_timing_trigger(60, 6);
		}
		break;
	}

	digi_play_loop("101_001", 3, 200, -1);
}

void Room101::daemon() {
	if (player_commands_allowed() && _G(roomVal2) && INTERFACE_VISIBLE) {
		player_update_info(_G(my_walker), &_G(player_info));

		if (_G(player_info).y > 374 && player_said("old bridge")) {
			player_set_commands_allowed(false);
			pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 0, 30, 1008);
			_G(kernel).call_daemon_every_loop = false;

		} else if (_G(player_info.y < 205) && player_said("town hall")) {
			player_set_commands_allowed(false);
			pal_fade_init(_G(master_palette), 16, 255, 0, 30, 1001);
			_G(kernel).call_daemon_every_loop = false;
		}
	}

	switch (_G(kernel).trigger) {
	case 1:
		if (_val2 == 12) {
			int frame = imath_ranged_rand(8, 10);
			series_play_("101ha01", 3840, 0, 1, 6, 0, 100, 0, 0, frame, frame);
		} else {
			digi_preload("101_002");
			series_play_("101ha01", 3840, 0, 21, 6, 0, 100, 0, 0, 11, 13);
		}
		break;

	case 2:
		digi_stop(2);
		unloadSounds();
		player_set_commands_allowed(true);
		ws_unhide_walker(_G(my_walker));
		break;

	case 4:
		digi_play(Common::String::format("101_0%d", imath_ranged_rand(10, 17)).c_str(), 2, 255, -1);
		break;

	case 5:
		digi_play("101_017", 2, 255, -1);
		break;

	case 6:
		if (!_G(flags)[ROOM101_FLAG2]) {
			uint idx = _G(flags)[ROOM101_FLAG3];
			assert(idx < 8);

			static const char *const NAMES[8] = {
				"101w500", "101w500", "101w501", "101w502",
				"101w503", "101w504", "101w505", "101w506"
			};

			wilbur_speech(NAMES[idx], 23);

		} else if (!_G(flags)[ROOM101_FLAG4]) {
			if (_G(flags)[ROOM101_FLAG5] <= 1) {
				wilbur_speech("101w520", 7);

			} else if (_G(flags)[ROOM101_FLAG6] && !_G(flags)[ROOM101_FLAG7]) {
				wilbur_speech("101w522", 7);
				_G(flags)[ROOM101_FLAG7] = 1;

			} else if (_G(flags)[ROOM101_FLAG8] && !_G(flags)[ROOM101_FLAG9]) {
				wilbur_speech("101w521", 7);
				_G(flags)[ROOM101_FLAG9] = 1;

			} else {
				wilbur_speech("101w524", 7);
			}
		} else if (!_G(flags)[ROOM101_FLAG10]) {
			if (_G(flags)[ROOM101_FLAG11] <= 1) {
				wilbur_speech("101w530", 7);
			} else if (!_G(flags)[ROOM101_FLAG12] && !_G(flags)[ROOM101_FLAG13]) {
				wilbur_speech("101w531", 7);
				_G(flags)[ROOM101_FLAG13] = 1;
			} else if (_G(flags)[ROOM101_FLAG12] && !_G(flags)[ROOM101_FLAG14]) {
				wilbur_speech("101w533", 7);
				_G(flags)[ROOM101_FLAG14] = 1;
			} else if (_G(flags)[ROOM101_FLAG12] && _G(flags)[ROOM101_FLAG11] >= 5 &&
					!_G(flags)[ROOM101_FLAG15]) {
				wilbur_speech("101w534", 7);
				_G(flags)[ROOM101_FLAG15] = 1;
			} else {
				wilbur_speech("101w532", 7);
			}
		} else if (!_G(flags)[ROOM101_FLAG16]) {
			switch (_G(flags)[ROOM101_FLAG17]) {
			case 0:
			case 1:
				wilbur_speech("101w550", 7);
				break;
			case 2:
				wilbur_speech("101w552", 7);
				break;
			default:
				if (_G(flags)[ROOM101_FLAG18] && !_G(flags)[ROOM101_FLAG19]) {
					wilbur_speech("101w551", 7);
					_G(flags)[ROOM101_FLAG19] = 1;
				} else {
					kernel_timing_trigger(60, 7);
				}
				break;
			}
		} else if (!_G(flags)[ROOM101_FLAG20]) {
			switch (_G(flags)[ROOM101_FLAG21]) {
			case 0:
			case 1:
				wilbur_speech("101w570", 7);
				break;
			case 2:
				wilbur_speech("101w571", 7);
				break;
			default:
				kernel_timing_trigger(60, 7);
				break;
			}
		} else {
			kernel_trigger_dispatch_now(7);
		}
		break;

	// TODO: cases
	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Room101::pre_parser() {
	bool lookAt = player_said_any("look", "look at");

	if (player_said("vera's diner") && !player_said_any("enter", "gear", "look", "look at"))
		player_hotspot_walk_override_just_face(9);

	if (player_said("alley") && !player_said_any("enter", "gear", "look", "look at"))
		player_hotspot_walk_override_just_face(3);

	_G(kernel).call_daemon_every_loop = player_said("ENTER", "OLD BRIDGE") ||
		player_said("gear", "old bridge") ||
		player_said("old bridge") ||
		player_said("ENTER", "TOWN HALL") ||
		player_said("gear", "town hall") ||
		(lookAt && player_said("town hall"));
}

void Room101::parser() {

}

void Room101::door() {
	_doorMachine = series_play_("101door", 3840, 0, -1, 10, -1, 100, 0, -53, 0, 0);
}

void Room101::unloadSounds() {
	digi_unload("101_010");
	digi_unload("101_011");
	digi_unload("101_012");
	digi_unload("101_013");
	digi_unload("101_014");
	digi_unload("101_015");
	digi_unload("101_016");
	digi_unload("101_017");
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
