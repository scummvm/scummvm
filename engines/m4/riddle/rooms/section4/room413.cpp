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

#include "m4/riddle/rooms/section4/room413.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room413::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room413::init() {
	_buttonFlag = false;
	interface_hide();
	player_set_commands_allowed(false);
	kernel_timing_trigger(1, 25);
	_G(kernel).call_daemon_every_loop = true;
}

void Room413::daemon() {
	switch (_G(kernel).trigger) {
	case 25:
		kernel_timing_trigger(30, 27);
		break;

	case 27:
		digi_play(_G(game).previous_room == 301 ? "413r02" : "413r01", 1, 255, 30);
		break;

	case 30:
		kernel_timing_trigger(30, 50);
		break;

	case 50:
		disable_player_commands_and_fade_init(75);
		break;

	case 75:
		interface_show();
		restoreAutosave();
		break;

	default:
		break;
	}

	if (_G(MouseState).ButtonState) {
		if (!_buttonFlag) {
			_G(MouseState).Event = 0;
			_G(kernel).call_daemon_every_loop = false;
			kernel_timing_trigger(1, 50);
		}

		_buttonFlag = true;
	}
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
