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

#include "m4/riddle/rooms/section7/room711.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room711::preload() {
	_G(player).walker_type = WALKER_ALT;
	_G(player).shadow_type = SHADOW_ALT;
	LoadWSAssets("OTHER SCRIPT", _G(master_palette));
}

void Room711::init() {
	digi_preload("711R03", -1);
	if (_G(flags[V224])) {
		hotspot_set_active(_G(currentSceneDef).hotspots, "MASTER LU'S BOOK", false);
		hotspot_set_active(_G(currentSceneDef).hotspots, "SIKKIMESE LU'S BOOK", false);
	}

	if (_G(game).previous_room == 710) {
		_711Rp01Series = series_load("711RP01", -1, nullptr);
		ws_demand_location(_G(my_walker), 245, 354);
		ws_demand_facing(_G(my_walker), 9);
		ws_hide_walker(_G(my_walker));
		kernel_timing_trigger(10, 100, nullptr);
	}
}

void Room711::pre_parser() {
	// No Implementation
}

void Room711::parser() {
	// TODO No implementation yet
}

void Room711::daemon() {
	switch (_G(kernel).trigger) {
	case 100:
		series_play("711RP01", 0, 0, 101, 0, 0, 100, 0, 0, 0, -1);
		break;

	case 101:
		series_unload(_711Rp01Series);
		player_set_commands_allowed(true);
		ws_unhide_walker(_G(my_walker));

		break;

	default:
		break;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
