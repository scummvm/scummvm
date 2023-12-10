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

#include "m4/burger/rooms/section4/section4_room.h"
#include "m4/burger/rooms/section4/section4.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

void Section4Room::poofArrive(int ticks) {
	player_set_commands_allowed(false);
	player_update_info();

	_G(player_dest_x) = _G(player_info).x;
	_G(player_dest_y) = _G(player_info).y;

	ws_demand_location(-100, _G(player_dest_y));
	ws_unhide_walker();
	kernel_timing_trigger(ticks, kPOOF);
}

void Section4Room::poof(int trigger) {
	Section4::poof(trigger);
}

} // namespace Rooms
} // namespace Burger
} // namespace M4
