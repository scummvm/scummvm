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

#include "m4/burger/rooms/section1/room171.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

void Room171::init() {
	_flag1 = false;
	_val1 = 0;

	if (_G(flags)[V092]) {
		hotspot_set_active("AUNT POLLY", false);
	} else {
		switch (_G(game).previous_room) {
		case 170:
			player_set_commands_allowed(false);

			if (_G(flags)[V084] && _G(flags)[V085] && _G(flags)[V086]) {
				loadSeries1();
				_val2 = 26;
				kernel_trigger_dispatch_now(5);
			} else {
				while (!_val1) {
					switch (imath_ranged_rand(1, 4)) {
					case 1:
						if (!_G(flags)[V083]) {
							_val1 = 10019;
							loadSeries2();
							_val2 = 5;
							kernel_trigger_dispatch_now(5);
						}
						break;

					case 2:
						_val1 = 10021;
						loadSeries3();
						digi_preload("171_002");
						digi_play_loop("171_002", 2);
						break;

					case 3:
						if (!_G(flags)[V085]) {
							_val1 = 10020;
							loadSeries4();

							if (imath_rand_bool(2)) {
								digi_preload("171_013a");
								digi_play_loop("171_013a", 2, 255);
							} else {
								digi_preload("171_013b");
								digi_play_loop("171_013b", 2, 255);
							}
						}
						break;

					case 4:
						if (!_G(flags)[V086]) {
							_val1 = 10022;
							loadSeries5();
							_val2 = 20;
							kernel_trigger_dispatch_now(5);
						}
						break;

					default:
						break;
					}
				}
			}
			break;

		case 175:
			if (!_G(flags)[V092]) {
				if (_G(flags)[V086]) {
					loadSeries1();
					_val2 = 26;
					kernel_trigger_dispatch_now(5);
				} else {
					_val1 = 10022;
					loadSeries5();
					kernel_trigger_dispatch_now(6);
				}
			}
			break;

		default:
			loadSeries1();
			_val2 = 26;
			kernel_trigger_dispatch_now(5);
			break;
		}
	}

	_series3 = series_show("171pills", 0x700);

	if (_val1 || _G(flags)[V092]) {
		_chair1 = series_show("171chair", 0x900);
		_chair2 = series_show("171chars", 0x901);
	}

	hotspot_set_active("PIANO STOOL", false);
	hotspot_set_active("PIANO STOOL ", false);

	if (_val1 == 10020 || _val1 == 10019) {
		hotspot_set_active("PIANO STOOL ", true);
		intr_add_no_walk_rect(378, 300, 518, 354, 377, 355, _G(screenCodeBuff)->get_buffer());

		if (_val1 == 10020) {
			series_show("171stool", 0x300);
			series_show("171stols", 0x301);
		}
	} else {
		series_show("171st02", 0x100);
		hotspot_set_active("PIANO STOOL", true);
	}

	_lid = series_show("171plid", 0);

	if (_val1 != 10022)
		series_show("171phone", 0x700);

	switch (_G(game).previous_room) {
	case 170:
		ws_demand_location(84, 275, 3);

		if (_G(flags)[V092] || (_G(flags)[V083] && _G(flags)[V084] &&
				_G(flags)[V085] && _G(flags)[V086])) {
			ws_walk(134, 275, nullptr, -1);
		} else {
			ws_walk(134, 275, nullptr, 6);
		}
		break;

	case 172:
		ws_demand_location(639, 289, 9);
		ws_walk(541, 295, nullptr, -1);
		break;

	case 175:
		ws_demand_location(237, 232, 9);
		_G(wilbur_should) = 2;
		kernel_trigger_dispatch_now(gCHANGE_WILBUR_ANIMATION);
		break;

	default:
		ws_demand_location(324, 319, 5);
		break;
	}
}

void Room171::daemon() {
}

void Room171::pre_parser() {
}

void Room171::parser() {
}

void Room171::loadSeries1() {
	series_load("171ap11");
	series_load("171ap12");
	series_load("171ap13");
	series_load("171ap11s");
	series_load("171ap12s");
	series_load("171ap13s");
	digi_preload("171_009");
}

void Room171::loadSeries2() {
	series_load("171ap01");
	series_load("171ap02");
	series_load("171ap03");
	series_load("171ap01s");
	series_load("171ap02s");
	series_load("171ap03s");
	loadSeries1();
}

void Room171::loadSeries3() {
	series_load("171ap04");
	series_load("171ap05");
	series_load("171ap06");
	series_load("171ap04s");
	series_load("171ap05s");
	series_load("171ap06s");
	loadSeries1();
}

void Room171::loadSeries4() {
	series_load("171ap07");
	series_load("171ap08");
	series_load("171ap09");
	series_load("171ap09s");
	loadSeries1();
}

void Room171::loadSeries5() {
	series_load("171ap10");
	series_load("171ap10s");
	loadSeries1();
}

void Room171::freeSeries() {
	if (_series1)
		terminateMachineAndNull(_series1);
	if (_series2)
		terminateMachineAndNull(_series2);
}


} // namespace Rooms
} // namespace Burger
} // namespace M4
