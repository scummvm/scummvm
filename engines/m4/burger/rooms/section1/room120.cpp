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

#include "m4/burger/rooms/section1/room120.h"
#include "m4/burger/rooms/section1/section1.h"
#include "m4/burger/vars.h"
#include "m4/graphics/gr_series.h"

namespace M4 {
namespace Burger {
namespace Rooms {

void Room120::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room120::init() {
	player_set_commands_allowed(false);
	kernel_timing_trigger(200, 1);
}

void Room120::daemon() {
	// After reimplementing for ScummVM the massive mess of multi-level switch
	// statements that are the previous rooms' daemons, I seriously love
	// the simplicity of this room's daemon. It's awesome!
	if (_G(kernel).trigger == 1) {
		_G(game).new_room = _G(game).previous_room;
	} else {
		_G(kernel).continue_handling_trigger = true;
	}
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
