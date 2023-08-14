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

#include "m4/burger/rooms/section1/room142.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

void Room142::init() {
	_G(player).walker_in_this_scene = true;
	digi_preload("142_004");
	digi_play_loop("142_004", 3, 255, -1);

	switch (_G(game).previous_room) {
	case RESTORING_GAME:
		if (_G(flags)[V059]) {
			ws_hide_walker();
			_G(roomVal1) = 2;
			kernel_trigger_dispatch_now(gTELEPORT);
		}
		break;

	case 101:
		ws_demand_location(120, 400, 2);
		ws_walk(120, 344, 0, -1, -1);
		break;

	case 139:
		ws_demand_location(-40, 375, 2);
		ws_walk(25, 344, 0, -1, -1);
		break;

	case 143:
		ws_demand_location(350, 270, 8);
		ws_walk(297, 275, 0, -1, -1);
		break;

	case 145:
		ws_demand_location(293, 275, 7);
		ws_walk(282, 280, 0, -1, -1);
		break;

	case 170:
		ws_demand_location(680, 325, 9);
		ws_walk(613, 331, 0, -1, -1);
		break;

	default:
		ws_demand_location(270, 320);
		ws_demand_facing(5);
		break;
	}

	series_play("142sm01", 0xf00, 4, -1, 6, -1);
	_series1 = series_show("142door", 0xe00);

	if (inv_player_has("PANTYHOSE")) {
		hotspot_set_active("FANBELT", false);
		hotspot_set_active("ENGINE", true);
	} else {
		hotspot_set_active("FANBELT", true);
		hotspot_set_active("ENGINE", false);
	}

	hotspot_set_active("TRUCK", false);

	if (_G(flags)[V000] == 1003) {
		_noWalk = intr_add_no_walk_rect(230, 250, 294, 277, 229, 278,
			_G(screenCodeBuff)->get_buffer());
		_series2 = series_show("142dt01", 0xd00, 0, -1, -1, 22);
		_series3 = series_show("142dt01s", 0xd01, 0, -1, -1, 22);
		hotspot_set_active("TRUCK", true);
	}

	hotspot_set_active("TOUR BUS", false);

	if (_G(flags)[V058]) {
		_series4 = series_show("142ba01", 0xf00, 0, -1, -1, 21);
		hotspot_set_active("TOUR BUS", true);
	}

	if (!_G(flags)[V059]) {
		_series5 = series_show("142icedr", 0xe00);
		hotspot_set_active("ICE", false);
	}
}

void Room142::daemon() {
}

void Room142::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	// TODO
}

void Room142::parser() {
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
