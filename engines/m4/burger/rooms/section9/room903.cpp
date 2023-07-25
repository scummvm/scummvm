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

#include "m4/burger/rooms/section9/room903.h"
#include "m4/adv_r/adv_control.h"
#include "m4/adv_r/adv_trigger.h"
#include "m4/graphics/gr_series.h"
#include "m4/burger/vars.h"
#include "m4/m4.h"

namespace M4 {
namespace Burger {
namespace Rooms {

void Room903::preload() {
	_G(player).walker_in_this_scene = false;
}

void Room903::init() {
	player_set_commands_allowed(false);
	_num1 = 0;
	_num2 = 0;
	_val1 = -1;
	_val2 = -1;
	_val3 = -1;

	series_show_("903logo", 0, 0, -1, -1, 0, 100, 386, 20);

	if (_G(game).previous_room <= 0 || _G(game).previous_room == 951) {
		kernel_trigger_dispatch_now(11);

	} else {
		if (!digi_play_state(1))
			kernel_trigger_dispatch_now(1);

		kernel_trigger_dispatch_now(2);
		kernel_trigger_dispatch_now(14);
		pal_fade_init(_G(master_palette), _G(kernel).first_fade, 255, 100, 60, -1);
	}
}

void Room903::daemon() {
	if (!_G(menu).menuSystemInitialized)
		return;

	switch (_G(kernel).trigger) {
	case 1:
		digi_preload("903music");
		digi_play("903music", 3, 100, -1);
		break;

	case 2:
		if (!_val1) {
			digi_preload("901click");

			for (int i = 0; i < _val2; ++i) {

			}
		}

		// TODO
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}


} // namespace Rooms
} // namespace Burger
} // namespace M4
