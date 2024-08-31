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

#include "m4/riddle/rooms/section4/room408.h"
#include "m4/riddle/rooms/section4/section4.h"
#include "m4/graphics/gr_series.h"
#include "m4/riddle/vars.h"

namespace M4 {
namespace Riddle {
namespace Rooms {

void Room408::init() {
	player_set_commands_allowed(false);

	if (_G(game).previous_room != KERNEL_RESTORING_GAME) {
		_val1 = 0;
		_val2 = -1;
		_val3 = 0;
		_val4 = -1;
		_val5 = -1;
		_val6 = 0;
		_val7 = 0;
		_val8 = 0;
		_val9 = 0;
	}

	digi_preload("950_s19");
	digi_preload("950_s20");
	digi_preload("950_s21");
	hotspot_set_active("WOLF", false);
	hotspot_set_active("PLANK", false);
	hotspot_set_active("EDGER", false);
	_exit = series_show("RIP EXITS 407", 0xf00, 16);

	if (_G(flags)[V139] == 1) {
		_G(flags)[V139] = 0;
		_edger = series_place_sprite("Edger gone", 0, 0, -53, 100, 0xf00);
		hotspot_set_active("EDGER", true);
		inv_move_object("EDGER", 408);
		ws_demand_location(234, 319, 3);
		ws_walk(438, 325, nullptr, 400, 1);
		
	} else if (_G(flags)[V139] == 3) {
		_G(flags)[V139] = 0;

		if (inv_object_is_here("PLANK")) {
			_plank = series_place_sprite("Plank gone", 0, 0, 0, 100, 0xf00);
			hotspot_set_active("PLANK", true);
		}

		ws_demand_location(-20, 345, 3);
		ws_walk(234, 319, nullptr, 420, 1);

	} else {
		if (inv_player_has("TURTLE") && !inv_player_has("EDGER") &&
				!_G(flags)[GLB_TEMP_12]) {
			_edger = series_place_sprite("Edger gone", 0, 0, -53, 100, 0xf00);
			hotspot_set_active("EDGER", true);
		}

		if (inv_player_has("TURTLE") && !inv_player_has("PLANK") &&
				!_G(flags)[GLB_TEMP_12] && inv_object_is_here("PLANK")) {
			_plank = series_place_sprite("Plank gone", 0, 0, 0, 100, 0xf00);
			hotspot_set_active("PLANK", true);
		}

		switch (_G(game).previous_room) {
		case KERNEL_RESTORING_GAME:
			digi_preload("950_s22");

			if (_G(flags)[V131] == 400) {
				hotspot_set_active("WOLF", true);
				_val8 = 2001;
				_val9 = 2200;
				_wolf = series_load("WOLF CLPNG LOOP LOOKS TO SIDE");
				_wolfie = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xd00, 0,
					triggerMachineByHashCallbackNegative, "WOLFIE");
				sendWSMessage_10000(1, _wolfie, _wolf, 1, 10, 110, _wolf, 10, 10, 0);
			}

			player_set_commands_allowed(true);
			break;

		case 402:
			if (_G(flags)[V132]) {
				ws_hide_walker();
				ws_walk_load_shadow_series(S4_SHADOW_DIRS, S4_SHADOW_NAMES);
				ws_walk_load_walker_series(S4_NORMAL_DIRS, S4_NORMAL_NAMES);
				kernel_timing_trigger(1, 300);
			} else {
				if (_G(flags)[V131] == 408) {
					hotspot_set_active("WOLF", true);
					_wolf = series_load("WOLF CLPNG LOOP LOOKS TO SIDE");
					_wolfie = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xd00, 0,
						triggerMachineByHashCallbackNegative, "WOLFIE");
					sendWSMessage_10000(1, _wolfie, _wolf, 1, 10, 110, _wolf, 10, 10, 0);
					_val8 = 2001;
					_val9 = 2200;
				}

				ws_demand_location(-20, 345, 3);
				ws_walk(35, 345, nullptr, 20, 3);
			}
			break;

		case 407:
			digi_preload("950_s22");
			terminateMachineAndNull(_exit);

			if (_G(flags)[GLB_TEMP_12]) {
				_G(flags)[V131] = 999;
			} else if (inv_player_has("TURTLE")) {
				_G(flags)[V131] = 402;
				_G(flags)[V117] = 1;
			} else if (_G(flags)[V124]) {
				_G(flags)[V131] = 402;
			} else {
				_G(flags)[V131] = 403;
			}

			_ripExits = series_load("RIP EXITS 407");
			ws_demand_location(201, 287, 4);
			ws_hide_walker();

			_exit = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, 0, 100, 0, 0,
				triggerMachineByHashCallbackNegative, "RIP ENTERS from GIZMO");
			sendWSMessage_10000(1, _exit, _ripExits, 1, 75, 40, _ripExists, 75, 75, 0);
			digi_play("408_s01", 2);
			break;

		default:
			digi_preload("950_s22");

			if (_G(flags)[V117] && _G(flags)[V125] == 3 &&
				!_G(flags)[GLB_TEMP_12] && !inv_player_has("EDGER") &&
				!inv_player_has("PLANK")) {
				_G(flags)[V131] = 408;
				_G(flags)[V117] = 0;
			}

			if (_G(flags)[V131] == 408) {
				hotspot_set_active("WOLF", true);
				_wolf = series_load("WOLF CLPNG LOOP LOOKS TO SIDE");
				_wolfie = TriggerMachineByHash(1, 1, 0, 0, 0, 0, 0, -53, 100, 0xd00, 0,
					triggerMachineByHashCallbackNegative, "WOLFIE");
				sendWSMessage_10000(1, _wolfie, _wolf, 1, 10, 110, _wolf, 10, 10, 0);
				_val8 = 2001;
				_val9 = 2200;
			}

			ws_demand_location(660, 345, 9);

			if (_G(flags)[V125] == 3) {
				series_simple_play("408 turtle popup", 0, true);
				ws_walk(438, 325, nullptr, 350, 1);
			} else {
				ws_walk(615, 345, nullptr, 30, 9);
			}
			break;
		}
	}

	digi_play_loop("950_s22", 3, 120, -1, 950);
}

void Room408::daemon() {
}

} // namespace Rooms
} // namespace Riddle
} // namespace M4
