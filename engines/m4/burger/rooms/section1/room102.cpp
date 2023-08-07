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

#include "m4/burger/rooms/section1/room102.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

void Room102::init() {
	_G(player).walker_in_this_scene = true;
	_val1 = 0;
	_val2 = 0;
	_val3 = -1;
	_val4 = 0;
	_val5 = 0;
	_val6 = 0;
	_val7 = 0;
	_val8 = 0;
	_val9 = 0;
	_val10 = 99999;

	digi_preload("102_001");
	digi_preload("102_002");

	switch (_G(flags)[V012]) {
	case 1:
		_index1 = series_load("102ha01", -1);
		_index2 = series_load("102ha03", -1);
		_val11 = 35;
		_val12 = 35;
		kernel_trigger_dispatch_now(3);

		_val13 = 58;
		_val14 = 62;
		kernel_trigger_dispatch_now(4);

		hotspot_set_active("harry", false);
		setup(5, 0);
		break;

	case 2:
		hotspot_set_active("harry", false);
		hotspot_set_active("harry ", false);

		_series1 = series_play_("102seat", 2560, 0, -1, 100, -1, 100, 0, 0, 0, 0);
		_series2 = series_play_("102seats", 2561, 0, -1, 100, -1, 100, 0, 0, 0, 0);
		setup(5, 0);
		break;

	default:
		_index1 = series_load("102ha01", -1);
		setup(0, 0);

		_val11 = 19;
		_val12 = 19;
		kernel_trigger_dispatch_now(3);
		++_G(flags)[V015];

		_series2 = series_play_("102seats", 2561, 0, -1, 100, -1, 100, 0, 0, 0, 0);
		hotspot_set_active("harry ", false);
		break;
	}

	_val15 = -1;

	if (_G(game).previous_room == -2) {
		ws_demand_location(_G(my_walker), 321, 343);
		ws_demand_facing(_G(my_walker), 4);
		ws_hide_walker(_G(my_walker));

	} else {
		_G(roomVal1) = 1;
		_val16 = 1;
		digi_preload("102_038");
		digi_play("102_038", 2, 255, gTELEPORT);
		player_set_commands_allowed(false);
	}

	kernel_trigger_dispatch_now(6);
	setupWax();
	setupLaz();
}

void Room102::daemon() {
}

void Room102::pre_parser() {
}

void Room102::parser() {
}

void Room102::setup(int val1, int val2) {
	digi_stop(3);

	if (val2)
		digi_play_loop("102_001", 3, 255, -1);
	if (_val1) {
		digi_unload(getDigi1(_G(flags)[GLB_TEMP_2]));
		digi_unload(getDigi2(_G(flags)[GLB_TEMP_2]));
	}

	if (val1) {
		_G(flags)[GLB_TEMP_2] = val1;
	} else {
		if (++_G(flags)[GLB_TEMP_2] == 36)
			_G(flags)[GLB_TEMP_2] = 1;
	}

	_val1 = 1;
	digi_preload(getDigi1(_G(flags)[GLB_TEMP_2]));
	digi_preload(getDigi2(_G(flags)[GLB_TEMP_2]));

	if (val2)
		digi_stop(3);

	digi_play_loop(getDigi2(_G(flags)[GLB_TEMP_2]), 3, 255, -1);
	_val10 = timer_read_60() + digi_ticks_to_play(getDigi2(_G(flags)[GLB_TEMP_2]));
	_val8 = 0;
}

void Room102::setupWax() {
	if (_G(flags)[V019]) {
		hotspot_set_active("hair wax   ", false);
		hotspot_set_active("hair wax  ", true);
		hotspot_set_active("hair wax ", true);
		hotspot_set_active("hair wax", false);
	} else {
		hotspot_set_active("hair wax   ", true);
		hotspot_set_active("hair wax  ", false);
		hotspot_set_active("hair wax ", false);
		hotspot_set_active("hair wax", true);
	}
}

void Room102::setupLaz() {
	if (_G(flags)[V019]) {
		_laz1 = series_play_("102lazys", 512, 0, -1, 100, -1, 100, 0, 0, 0, 0);
		_laz2 = series_play_("102lazss", 513, 0, -1, 100, -1, 100, 0, 0, 0, 0);
	} else {
		_laz1 = series_play_("102lazy2", 512, 0, -1, 100, -1, 100, 0, 0, 0, 0);
		_laz2 = series_play_("102laz2s", 513, 0, -1, 100, -1, 100, 0, 0, 0, 0);
	}
}

const char *Room102::getDigi1(int num) const {
	static const char *NAMES[34] = {
		"102h0323", "102h032b", "102h032c", "102h032d",
		"102h032e", "102h032f", "102h032g", "102h032h",
		"102h032i", "102h032j", "102h032k", "102h032l",
		"102h032m", "102h0321", "102h032o", "102h032p",
		"102h032q", "102h032r", "102h032s", "102h0324",
		"102h032u", "102h0321", "102h032w", "102h032x",
		"102h032y", "102h032z", "102h032v", "102h0322",
		"102h032a", "102h032t", "102h0325", "102h0326",
		"102h0327", "102h0328" };

	return (num >= 1 && num <= 34) ? NAMES[num - 1] : "102h0329";
}

const char *Room102::getDigi2(int num) const {
	static const char *NAMES[35] = {
		"102_011", "102_004", nullptr,   "102_006", "102_008",
		"102_009", "102_013", "102_005", "102_014", "102_023",

		"102_020", "102_011", "102_015", "102_006", "102_007",
		"102_016", "102_014", "102_015", "102_013", "102_011",

		"102_003", "102_012", "102_008", "102_013", "102_021",
		"102_011", "102_013", "102_010", "102_003", "102_005",

		"102_010", "102_011", "102_021", "102_021", "102_012"
	};

	return (num >= 1 && num <= 35) ? NAMES[num - 1] : "102_024";
}


} // namespace Rooms
} // namespace Burger
} // namespace M4
